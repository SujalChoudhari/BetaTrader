#include "fix_client/FixClientSession.h"
#include "common_fix/OutboundMessageBuilder.h"
#include "common_fix/FixUtils.h"
#include "common_fix/Protocol.h"
#include "logging/Logger.h"

#include <iostream>

namespace fix_client {

    FixClientSession::FixClientSession(asio::io_context& ioContext, 
                                       const std::string& senderCompId,
                                       const std::string& targetCompId,
                                       const std::string& seqStoreDir)
        : mIoContext(ioContext),
          mSocket(ioContext),
          mResolver(ioContext),
          mHeartbeatTimer(ioContext),
          mTestReqTimer(ioContext),
          mSenderCompId(senderCompId),
          mTargetCompId(targetCompId),
          mSeqStore(senderCompId, seqStoreDir),
          mReadChunk(ChunkSize)
    {
    }

    FixClientSession::~FixClientSession() {
        disconnect();
    }

    void FixClientSession::connect(const std::string& host, short port) {
        changeState(FixClientState::Connecting);
        
        auto self(shared_from_this());
        mResolver.async_resolve(host, std::to_string(port),
            [this, self](const std::error_code& ec, asio::ip::tcp::resolver::results_type results) {
                if (!ec) {
                    asio::async_connect(mSocket, results,
                        [this, self](const std::error_code& ec, const asio::ip::tcp::endpoint& endpoint) {
                            if (!ec) {
                                LOG_INFO("FixClientSession connected to {}:{}", endpoint.address().to_string(), endpoint.port());
                                changeState(FixClientState::Connected);
                                doRead();
                            } else {
                                LOG_ERROR("FixClientSession failed to connect: {}", ec.message());
                                changeState(FixClientState::Disconnected);
                            }
                        });
                } else {
                    LOG_ERROR("FixClientSession resolve failed: {}", ec.message());
                    changeState(FixClientState::Disconnected);
                }
            });
    }

    void FixClientSession::disconnect() {
        if (mState != FixClientState::Disconnected) {
            std::error_code ec;
            mHeartbeatTimer.cancel();
            mTestReqTimer.cancel();
            mSocket.close(ec);
            changeState(FixClientState::Disconnected);
            LOG_INFO("FixClientSession forcefully disconnected.");
        }
    }

    void FixClientSession::sendLogon(int heartbeatInterval, bool forceReset) {
        if (mState != FixClientState::Connected && mState != FixClientState::Disconnected) {
            LOG_WARN("Cannot send Logon, session isn't in Connected state (Current: {}).", static_cast<int>(mState.load()));
            return;
        }

        if (forceReset) {
            mSeqStore.reset();
            LOG_INFO("Client forced sequence reset prior to Logon.");
        }

        mHeartbeatInterval = heartbeatInterval;
        
        uint32_t outSeq = mSeqStore.getNextSenderSeqNum();
        std::string logonMsg = fix::OutboundMessageBuilder::buildLogon(mSenderCompId, mTargetCompId, outSeq, heartbeatInterval);
        
        if (forceReset) {
            // Need to insert ResetSeqNumFlag=Y (141=Y) before the end of the body
            // This is a minimal hack for `client_fix` without full builder support
            size_t insertPos = logonMsg.find("108=");
            if (insertPos != std::string::npos) {
                // Adjusting the BodyLength would be tedious here. 
                // A better approach is to use the robust buildMessage.
                std::ostringstream body;
                body << "98=0\x01" << "108=" << heartbeatInterval << '\x01' << "141=Y\x01";
                logonMsg = fix::OutboundMessageBuilder::buildMessage(mSenderCompId, mTargetCompId, outSeq, "A", body.str());
            }
        }
        
        mSeqStore.setSeqNums(mSeqStore.getNextTargetSeqNum(), outSeq + 1);
        doWrite(std::make_shared<std::string>(logonMsg));
        changeState(FixClientState::LogonSent);
        LOG_INFO("Sent Logon (35=A) to {}", mTargetCompId);
    }

    void FixClientSession::sendLogout(const std::string& reason) {
        if (mState == FixClientState::Active || mState == FixClientState::LogonSent) {
            uint32_t outSeq = mSeqStore.getNextSenderSeqNum();
            std::string logoutMsg = fix::OutboundMessageBuilder::buildLogout(mSenderCompId, mTargetCompId, outSeq, reason);
            mSeqStore.setSeqNums(mSeqStore.getNextTargetSeqNum(), outSeq + 1);
            
            doWrite(std::make_shared<std::string>(logoutMsg));
            changeState(FixClientState::LoggingOut);
            LOG_INFO("Sent Logout (35=5) to server: {}", reason);
        }
    }

    void FixClientSession::sendMessage(const std::string& msgType, const std::string& bodyStr) {
        if (mState != FixClientState::Active) {
            LOG_WARN("Attempted to send msgType {} but session is not Active.", msgType);
            return;
        }
        
        uint32_t outSeq = mSeqStore.getNextSenderSeqNum();
        std::string rawFix = fix::OutboundMessageBuilder::buildMessage(mSenderCompId, mTargetCompId, outSeq, msgType, bodyStr);
        mSeqStore.setSeqNums(mSeqStore.getNextTargetSeqNum(), outSeq + 1);
        
        doWrite(std::make_shared<std::string>(rawFix));
    }

    void FixClientSession::doWrite(std::shared_ptr<std::string> message) {
        auto self(shared_from_this());
        asio::async_write(mSocket, asio::buffer(*message),
            [this, self, message](const std::error_code& ec, std::size_t) {
                if (!ec) {
                    LOG_TRACE("Sent raw size {}: {}", message->length(), message->substr(0, 50));
                } else {
                    LOG_ERROR("FixClientSession Write error: {}", ec.message());
                    disconnect();
                }
            });
    }

    void FixClientSession::doRead() {
        auto self(shared_from_this());
        mSocket.async_read_some(asio::buffer(mReadChunk),
            [this, self](const std::error_code& ec, std::size_t length) {
                if (!ec) {
                    mReadBuffer.append(mReadChunk.begin(), mReadChunk.begin() + length);

                    size_t pos = 0;
                    while ((pos = mReadBuffer.find(std::string("8=FIX.4.4\x01" "9="))) != std::string::npos) {
                        size_t checksumEnd = mReadBuffer.find("10=", pos);
                        if (checksumEnd != std::string::npos) {
                            size_t endOfMessage = mReadBuffer.find(fix::SOH, checksumEnd);
                            if (endOfMessage != std::string::npos) {
                                endOfMessage++; // Include SOH
                                
                                std::string fullFixMessage = mReadBuffer.substr(pos, endOfMessage - pos);
                                handleProtocolMessage(fullFixMessage);
                                
                                mReadBuffer.erase(0, endOfMessage);
                                continue;
                            }
                        }
                        break; // Need more data
                    }
                    doRead();
                } else {
                    LOG_WARN("FixClientSession Read error: {}", ec.message());
                    disconnect();
                }
            });
    }

    void FixClientSession::handleProtocolMessage(const std::string& msgStr) {
        // Extract basic routing fields
        auto map = fix::splitToMap(msgStr, fix::SOH);
        
        uint32_t seqNum = 0;
        if (auto it = map.find(34); it != map.end()) seqNum = std::stoul(std::string(it->second));
        
        std::string msgTypeStr;
        if (auto it = map.find(35); it != map.end()) msgTypeStr = it->second;

        if (msgTypeStr.empty()) return;
        char msgType = msgTypeStr[0];

        // Sequence Number Validation (Server -> Client)
        uint32_t expectedSeq = mSeqStore.getNextTargetSeqNum();
        
        if (msgType != '4' && msgType != '5') { // SequenceReset and Logout don't increment target strictly if error
            if (seqNum == expectedSeq) {
                mSeqStore.setSeqNums(expectedSeq + 1, mSeqStore.getNextSenderSeqNum());
            } else if (seqNum > expectedSeq) {
                LOG_WARN("Sequence gap detected from server. Expected: {}, Got: {}", expectedSeq, seqNum);
                // Real implementation would send ResendRequest. For BetaTrader client_fix, we will just sync up to avoid death loops.
                mSeqStore.setSeqNums(seqNum + 1, mSeqStore.getNextSenderSeqNum());
            } else {
                LOG_ERROR("Fatal: Sequence number from server too low. Expected: {}, Got: {}", expectedSeq, seqNum);
                disconnect();
                return;
            }
        }

        // Protocol handlers
        switch (msgType) {
            case 'A': // Logon Ack
                handleLogonResponse(msgStr, seqNum);
                break;
            case '0': // Heartbeat
                handleHeartbeat(msgStr);
                break;
            case '1': // Test Request
                handleTestRequest(msgStr);
                break;
            case '2': // Resend Request
                handleResendRequest(msgStr);
                break;
            case '4': // Sequence Reset
                handleSequenceReset(msgStr);
                break;
            case '5': // Logout
                handleLogoutResponse(msgStr);
                break;
            default: {
                // Application-level message (ExecutionReport, MarketData, etc)
                ParsedFixMessage parsed = FixMessageParser::parse(msgStr);
                if (!std::holds_alternative<std::monostate>(parsed)) {
                    if (mMessageCb) mMessageCb(parsed);
                } else {
                    LOG_WARN("Received unparseable or irrelevant application message: {}", msgStr.substr(0, 50));
                }
                break;
            }
        }
    }

    void FixClientSession::handleLogonResponse(const std::string& msgStr, uint32_t inSeq) {
        LOG_INFO("Logon Acknowledged by server. Session is active.");
        changeState(FixClientState::Active);
        startHeartbeatTimer();
    }

    void FixClientSession::handleLogoutResponse(const std::string& msgStr) {
        LOG_INFO("Logout confirmed by server.");
        disconnect(); // Force TCP down cleanly
    }

    void FixClientSession::handleHeartbeat(const std::string& msgStr) {
        LOG_TRACE("Received Heartbeat (35=0) from server.");
        // We could track last received time here to detect dead server
    }

    void FixClientSession::handleTestRequest(const std::string& msgStr) {
        auto map = fix::splitToMap(msgStr, fix::SOH);
        std::string testReqId = "";
        if (auto it = map.find(112); it != map.end()) {
            testReqId = it->second;
        }
        
        uint32_t outSeq = mSeqStore.getNextSenderSeqNum();
        std::string hbMsg = fix::OutboundMessageBuilder::buildHeartbeat(mSenderCompId, mTargetCompId, outSeq, testReqId);
        mSeqStore.setSeqNums(mSeqStore.getNextTargetSeqNum(), outSeq + 1);
        
        doWrite(std::make_shared<std::string>(hbMsg));
        LOG_TRACE("Responded to TestRequest {} with Heartbeat.", testReqId);
    }

    void FixClientSession::handleResendRequest(const std::string& msgStr) {
        LOG_WARN("Server sent ResendRequest (35=2). Client does not cache outbound messages; sending SequenceReset-GapFill.");
        auto map = fix::splitToMap(msgStr, fix::SOH);
        
        uint32_t beginSeqNo = 0;
        if (auto it = map.find(7); it != map.end()) beginSeqNo = std::stoul(std::string(it->second));
        
        uint32_t endSeqNo = 0;
        if (auto it = map.find(16); it != map.end()) endSeqNo = std::stoul(std::string(it->second));
        
        uint32_t newSeqNo = mSeqStore.getNextSenderSeqNum(); // Tell server to expect our current seq
        
        std::ostringstream body;
        body << "123=Y\x01" << "36=" << newSeqNo << "\x01"; // GapFillFlag=Y, NewSeqNo
        
        // Use beginSeqNo as the MsgSeqNum of the GapFill message itself
        std::string resetMsg = fix::OutboundMessageBuilder::buildMessage(mSenderCompId, mTargetCompId, beginSeqNo, "4", body.str());
        
        doWrite(std::make_shared<std::string>(resetMsg));
    }

    void FixClientSession::handleSequenceReset(const std::string& msgStr) {
        auto map = fix::splitToMap(msgStr, fix::SOH);
        uint32_t newSeqNo = 0;
        if (auto it = map.find(36); it != map.end()) {
            newSeqNo = std::stoul(std::string(it->second));
            mSeqStore.setSeqNums(newSeqNo, mSeqStore.getNextSenderSeqNum());
            LOG_INFO("Server sent SequenceReset (35=4). Resetting TargetSeqNum to {}", newSeqNo);
        }
    }

    void FixClientSession::changeState(FixClientState newState) {
        mState = newState;
        if (mStateChangeCb) {
            mStateChangeCb(newState);
        }
    }

    void FixClientSession::startHeartbeatTimer() {
        if (mState != FixClientState::Active) return;

        mHeartbeatTimer.expires_after(std::chrono::seconds(mHeartbeatInterval));
        
        auto self(shared_from_this());
        mHeartbeatTimer.async_wait([this, self](const std::error_code& ec) {
            if (!ec && mState == FixClientState::Active) {
                uint32_t outSeq = mSeqStore.getNextSenderSeqNum();
                std::string hbMsg = fix::OutboundMessageBuilder::buildHeartbeat(mSenderCompId, mTargetCompId, outSeq);
                mSeqStore.setSeqNums(mSeqStore.getNextTargetSeqNum(), outSeq + 1);
                
                doWrite(std::make_shared<std::string>(hbMsg));
                
                startHeartbeatTimer(); // Reschedule
            }
        });
    }

    void FixClientSession::startTestRequestTimer() {
        // Advanced: Unimplemented. Can be added to ping server if quiet.
    }

    void FixClientSession::sendNewOrder(const std::string& symbol, char side, double price, int qty, char type, char tif) {
        std::ostringstream body;
        auto now = std::chrono::system_clock::now();
        auto clOrdId = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        body << "11=" << clOrdId << "\x01"      // ClOrdID
             << "55=" << symbol << "\x01"       // Symbol
             << "54=" << side << "\x01"         // Side
             << "60=" << fix::OutboundMessageBuilder::generateTimestamp() << "\x01" // TransactTime
             << "38=" << qty << "\x01"          // OrderQty
             << "40=" << type << "\x01";        // OrdType

        if (type != '1') {                      // Not Market
             body << "44=" << std::fixed << std::setprecision(5) << price << "\x01"; // Price
        }
        
        if (tif != '0') {
            body << "59=" << tif << "\x01";     // TimeInForce
        }

        sendMessage("D", body.str());
        LOG_INFO("Sent NewOrderSingle (35=D) for {} Side={} Qty={} Price={}", symbol, side, qty, price);
    }

    void FixClientSession::sendMarketDataRequest(const std::string& symbol, char subscriptionRequestType) {
        std::ostringstream body;
        auto mdReqId = "req_" + symbol + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

        body << "262=" << mdReqId << "\x01"     // MDReqID
             << "263=" << subscriptionRequestType << "\x01" // SubscriptionRequestType (1=Snapshot+Updates)
             << "264=0\x01"                     // MarketDepth (0=Full)
             << "267=2\x01"                     // NoMDEntryTypes (Bid, Offer)
             << "269=0\x01"                     // MDEntryType (Bid)
             << "269=1\x01"                     // MDEntryType (Offer)
             << "146=1\x01"                     // NoRelatedSym
             << "55=" << symbol << "\x01";      // Symbol

        sendMessage("V", body.str());
        LOG_INFO("Sent MarketDataRequest (35=V) for {} Type={}", symbol, subscriptionRequestType);
    }

} // namespace fix_client
