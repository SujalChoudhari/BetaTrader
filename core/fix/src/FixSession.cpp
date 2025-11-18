#include "fix/FixSession.h"
#include "common/Instrument.h"
#include "fix/BinaryToCancelOrderRequestConverter.h"
#include "fix/BinaryToMarketDataRequestConverter.h"
#include "fix/BinaryToModifyOrderRequestConverter.h"
#include "fix/BinaryToOrderRequestConverter.h"
#include "fix/ExecutionReportToBinaryConverter.h"
#include "fix/FixRunbookDefinations.h"
#include "fix/MarketDataIncrementalRefreshToBinaryConverter.h"
#include "fix/MarketDataSnapshotFullRefreshToBinaryConverter.h"
#include "common_fix/Protocol.h"
#include "logging/Runbook.h"
#include "trading_core/CancelOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/NewOrder.h"
#include <iostream>
#include <string_view>

namespace fix {


    FixSession::FixSession(asio::ip::tcp::socket socket,
                           trading_core::TradingCore& tradingCore,
                           const uint32_t sessionId)
        : mSocket(std::move(socket)), mTradingCore(tradingCore),
          mData(MaxLength), mSessionId(sessionId)
    {
        LOG_INFO("New FIX session created with ID: {}", mSessionId);
    }

    void FixSession::start()
    {
        doRead();
    }

    void FixSession::sendExecutionReport(const ExecutionReport& report)
    {
        auto binaryReport = ExecutionReportToBinaryConverter::convert(report);
        doWrite(binaryReport);
    }

    void FixSession::sendMarketDataSnapshotFullRefresh(const MarketDataSnapshotFullRefresh& snapshot)
    {
        auto binarySnapshot = MarketDataSnapshotFullRefreshToBinaryConverter::convert(snapshot);
        doWrite(binarySnapshot);
    }

    void FixSession::sendMarketDataIncrementalRefresh(const MarketDataIncrementalRefresh& refresh)
    {
        auto binaryRefresh = MarketDataIncrementalRefreshToBinaryConverter::convert(refresh);
        doWrite(binaryRefresh);
    }

    void FixSession::sendReject(const Reject& reject)
    {
        // TODO: Implement serialization for Reject message and send it.
        LOG_WARN("Placeholder: Would send session-level Reject to Session {}: {}", mSessionId, reject.text);
    }

    void FixSession::sendBusinessMessageReject(const BusinessMessageReject& bizReject)
    {
        // TODO: Implement serialization for BusinessMessageReject message and send it.
        LOG_WARN("Placeholder: Would send business-level Reject to Session {}: {}", mSessionId, bizReject.text);
    }

    uint32_t FixSession::getSessionID() const
    {
        return mSessionId;
    }

    void FixSession::doWrite(const std::string& message)
    {
        auto self(shared_from_this());
        asio::async_write(
            mSocket, asio::buffer(message),
            [this, self, message](const std::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    LOG_INFO("Sent FIX message to Session {}: {}", mSessionId, message.substr(0, 100));
                }
                else {
                    LOG_ERROR(errors::EFIX3, "Failed to send message to Session {}: {}", mSessionId, ec.message());
                }
            });
    }

    void FixSession::doRead()
    {
        auto self(shared_from_this());
        mSocket.async_read_some(asio::buffer(mData), [this, self](const std::error_code ec, const std::size_t length) {
            if (!ec) {
                mReadBuffer.append(mData.begin(), mData.begin() + length);

                size_t pos = 0;
                while ((pos = mReadBuffer.find(std::string(1, SOH) + "10=", pos)) != std::string::npos) {
                    size_t endOfMessage = mReadBuffer.find(SOH, pos + 4);
                    if (endOfMessage != std::string::npos) {
                        endOfMessage++;

                        std::string fullFixMessage = mReadBuffer.substr(0, endOfMessage);

                        size_t msgTypeStart = fullFixMessage.find("35=");
                        if (msgTypeStart != std::string::npos && msgTypeStart + 4 < fullFixMessage.length()) {
                            char msgType = fullFixMessage[msgTypeStart + 3];
                            handleFixMessage(fullFixMessage, msgType);
                        } else {
                            LOG_WARN("Could not extract MsgType from FIX message: {}", fullFixMessage.substr(0, 50));
                            // TODO: Send Reject (35=3) for malformed message or missing MsgType.
                        }

                        mReadBuffer.erase(0, endOfMessage);
                        pos = 0;
                    } else {
                        break;
                    }
                }
                doRead();
            }
            else {
                if (ec != asio::error::eof) {
                    LOG_ERROR(errors::EFIX3, "Error reading from socket for Session {}: {}", mSessionId, ec.message());
                }
                LOG_INFO("FIX Session {} disconnected.", mSessionId);
            }
        });
    }

    void FixSession::handleFixMessage(const std::string& fixMessage, char msgType)
    {
        try {
            switch (msgType) {
                case 'A': // Logon
                    // TODO: Implement Logon message handling.
                    LOG_INFO("Received Logon message from Session {}. Handling not yet implemented.", mSessionId);
                    break;
                case '5': // Logout
                    // TODO: Implement Logout message handling.
                    LOG_INFO("Received Logout message from Session {}. Handling not yet implemented.", mSessionId);
                    break;
                case '0': // Heartbeat
                    // TODO: Implement Heartbeat message handling.
                    LOG_INFO("Received Heartbeat message from Session {}. Handling not yet implemented.", mSessionId);
                    break;
                case '1': // Test Request
                    // TODO: Implement Test Request message handling.
                    LOG_INFO("Received Test Request message from Session {}. Handling not yet implemented.", mSessionId);
                    break;
                case '2': // Resend Request
                    // TODO: Implement Resend Request message handling.
                    LOG_INFO("Received Resend Request message from Session {}. Handling not yet implemented.", mSessionId);
                    break;
                case '4': // Sequence Reset
                    // TODO: Implement Sequence Reset message handling.
                    LOG_INFO("Received Sequence Reset message from Session {}. Handling not yet implemented.", mSessionId);
                    break;
                case MSG_TYPE_NEW_ORDER_SINGLE:
                    {
                        auto orderRequest = BinaryToOrderRequestConverter::convert(fixMessage);
                        if (orderRequest) {
                            LOG_INFO("Received NewOrderSingle from Session {}: ClOrdID={}, Symbol={}, Qty={}, Price={}",
                                     mSessionId, orderRequest->clientOrderId, common::to_string(orderRequest->symbol),
                                     orderRequest->quantity, orderRequest->price);

                            // TODO: Extract actual OrderType (Tag 40) and TimeInForce (Tag 59) from FIX message.
                            // Currently assuming Limit and DAY.
                            auto common_order = std::make_unique<common::Order>(
                                    orderRequest->clientOrderId, // Use the parsed uint64_t ID
                                    orderRequest->symbol, 
                                    std::to_string(mSessionId),
                                    orderRequest->side, 
                                    common::OrderType::Limit,
                                    common::TimeInForce::DAY,
                                    orderRequest->quantity,
                                    orderRequest->price,
                                    std::chrono::system_clock::now());

                            auto newOrderCmd = std::make_unique<trading_core::NewOrder>(
                                    common_order->getClientId(),
                                    common_order->getTimestamp(),
                                    std::move(common_order));

                            mTradingCore.submitCommand(std::move(newOrderCmd));
                        } else {
                            LOG_WARN("Failed to parse NewOrderSingle from Session {}.", mSessionId);
                            // TODO: Send BusinessMessageReject (35=j) for parsing failure.
                        }
                    }
                    break;
                case MSG_TYPE_ORDER_CANCEL_REQUEST:
                    handleCancelOrderRequest(fixMessage);
                    break;
                case MSG_TYPE_ORDER_CANCEL_REPLACE_REQUEST:
                    handleModifyOrderRequest(fixMessage);
                    break;
                case MSG_TYPE_MARKET_DATA_REQUEST:
                    handleMarketDataRequest(fixMessage);
                    break;
                default:
                    LOG_WARN("Received unhandled FIX message type '{}' from Session {}: {}", msgType, mSessionId, fixMessage.substr(0, 50));
                    // TODO: Send Reject (35=3) for unsupported MsgType.
                    break;
            }
        }
        catch (const std::exception& e) {
            LOG_ERROR(errors::EFIX2, "Exception processing FIX message from Session {}: {}", mSessionId, e.what());
            // TODO: Send Reject (35=3) or BusinessMessageReject (35=j) depending on error context.
        }
    }

    void FixSession::handleCancelOrderRequest(const std::string& fixMessage)
    {
        auto cancelRequest = BinaryToCancelOrderRequestConverter::convert(fixMessage);
        if (cancelRequest) {
            LOG_INFO("Received CancelOrderRequest from Session {}: ClOrdID={}, OrderID={}, Symbol={}",
                     mSessionId, cancelRequest->clOrdID, cancelRequest->orderID, common::to_string(cancelRequest->symbol));

            // Prefer OrderID (37) if present, otherwise use ClOrdID (11).
            common::OrderID orderIdToCancel = (cancelRequest->orderID != 0) ? cancelRequest->orderID : cancelRequest->clOrdID;

            if (orderIdToCancel == 0) {
                LOG_WARN("CancelOrderRequest from Session {} has no valid OrderID or ClOrdID.", mSessionId);
                // TODO: Send BusinessMessageReject (35=j) if neither OrderID nor ClOrdID is present.
                return;
            }

            auto cancelCmd = std::make_unique<trading_core::CancelOrder>(
                std::to_string(mSessionId),
                cancelRequest->transactTime,
                orderIdToCancel
            );
            mTradingCore.submitCommand(std::move(cancelCmd));
        } else {
            LOG_WARN("Failed to parse CancelOrderRequest from Session {}.", mSessionId);
            // TODO: Send BusinessMessageReject (35=j) for parsing failure.
        }
    }

    void FixSession::handleModifyOrderRequest(const std::string& fixMessage)
    {
        auto modifyRequest = BinaryToModifyOrderRequestConverter::convert(fixMessage);
        if (modifyRequest) {
            LOG_INFO("Received ModifyOrderRequest from Session {}: ClOrdID={}, OrigClOrdID={}, OrderID={}, Symbol={}, Qty={}, Price={}",
                     mSessionId, modifyRequest->clOrdID, modifyRequest->origClOrdID, modifyRequest->orderID,
                     common::to_string(modifyRequest->symbol), modifyRequest->orderQty, modifyRequest->price);

            // Per FIX spec, OrigClOrdID (41) is required. Some systems might use OrderID (37).
            common::OrderID orderIdToModify = (modifyRequest->origClOrdID != 0) ? modifyRequest->origClOrdID : modifyRequest->orderID;

            if (orderIdToModify == 0) {
                LOG_WARN("ModifyOrderRequest from Session {} has no valid OrigClOrdID or OrderID.", mSessionId);
                // TODO: Send BusinessMessageReject (35=j) if neither is present.
                return;
            }

            auto modifyCmd = std::make_unique<trading_core::ModifyOrder>(
                std::to_string(mSessionId),
                modifyRequest->transactTime,
                orderIdToModify,
                modifyRequest->price,
                modifyRequest->orderQty
            );

            mTradingCore.submitCommand(std::move(modifyCmd));

        } else {
            LOG_WARN("Failed to parse ModifyOrderRequest from Session {}.", mSessionId);
            // TODO: Send BusinessMessageReject (35=j) for parsing failure.
        }
    }

    void FixSession::handleMarketDataRequest(const std::string& fixMessage)
    {
        auto mdRequest = BinaryToMarketDataRequestConverter::convert(fixMessage);
        if (mdRequest) {
            LOG_INFO("Received MarketDataRequest from Session {}: MDReqID={}, Type={}, Depth={}, Symbols={}",
                     mSessionId, mdRequest->mdReqID, mdRequest->subscriptionRequestType,
                     mdRequest->marketDepth, mdRequest->symbols.empty() ? "N/A" : common::to_string(mdRequest->symbols[0]));

            // TODO: Integrate with actual market data system to provide real data.
            // This skeleton currently simulates a market data response.
            if (mdRequest->subscriptionRequestType == SUBSCRIPTION_REQUEST_TYPE_SNAPSHOT ||
                mdRequest->subscriptionRequestType == SUBSCRIPTION_REQUEST_TYPE_SNAPSHOT_AND_UPDATES)
            {
                MarketDataSnapshotFullRefresh snapshot;
                snapshot.targetSessionID = mSessionId;
                snapshot.mdReqID = mdRequest->mdReqID;
                if (!mdRequest->symbols.empty()) {
                    snapshot.symbol = mdRequest->symbols[0];
                } else {
                    snapshot.symbol = common::from_string("EURUSD");
                }

                MarketDataEntry bid1 = {MDEntryType::Bid, 100.0, 100, std::chrono::system_clock::now(), 1};
                MarketDataEntry offer1 = {MDEntryType::Offer, 101.0, 150, std::chrono::system_clock::now(), 1};
                snapshot.entries.push_back(bid1);
                snapshot.entries.push_back(offer1);

                sendMarketDataSnapshotFullRefresh(snapshot);

                if (mdRequest->subscriptionRequestType == SUBSCRIPTION_REQUEST_TYPE_SNAPSHOT_AND_UPDATES) {
                    // TODO: Implement continuous incremental market data updates.
                    // This would typically involve a dedicated publisher or timer-based mechanism.
                    LOG_INFO("Session {} subscribed to market data updates for MDReqID {}", mSessionId, mdRequest->mdReqID);
                }
            } else if (mdRequest->subscriptionRequestType == SUBSCRIPTION_REQUEST_TYPE_UNSUBSCRIBE) {
                LOG_INFO("Session {} unsubscribed from market data for MDReqID {}", mSessionId, mdRequest->mdReqID);
                // TODO: Implement actual unsubscription logic, removing the session from market data distribution lists.
            }
        } else {
            LOG_WARN("Failed to parse MarketDataRequest from Session {}.", mSessionId);
            // TODO: Send BusinessMessageReject (35=j) for parsing failure.
        }
    }

} // namespace fix
