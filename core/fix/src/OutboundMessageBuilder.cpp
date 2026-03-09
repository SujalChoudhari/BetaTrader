#include "fix/OutboundMessageBuilder.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <numeric>

namespace fix {

    std::string OutboundMessageBuilder::generateTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = std::chrono::system_clock::to_time_t(now);
        std::tm bt = *std::gmtime(&timer);

        std::ostringstream oss;
        oss << std::put_time(&bt, "%Y%m%d-%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    std::string OutboundMessageBuilder::buildMessage(const std::string& senderCompId, 
                                                     const std::string& targetCompId, 
                                                     uint32_t msgSeqNum, 
                                                     const std::string& msgType, 
                                                     const std::string& bodyStr) {
        std::ostringstream headerAndBody;
        
        // 35=MsgType, 49=SenderCompID, 56=TargetCompID, 34=MsgSeqNum, 52=SendingTime
        headerAndBody << "35=" << msgType << '\x01'
                      << "49=" << senderCompId << '\x01'
                      << "56=" << targetCompId << '\x01'
                      << "34=" << msgSeqNum << '\x01'
                      << "52=" << generateTimestamp() << '\x01'
                      << bodyStr;

        std::string payload = headerAndBody.str();
        
        std::ostringstream finalMsg;
        // 8=BeginString, 9=BodyLength
        finalMsg << "8=FIX.4.4\x01" << "9=" << payload.length() << '\x01' << payload;
        
        std::string msgBeforeChecksum = finalMsg.str();
        
        // Calculate Checksum (10)
        int sum = std::accumulate(msgBeforeChecksum.begin(), msgBeforeChecksum.end(), 0);
        int checksum = sum % 256;
        
        finalMsg << "10=" << std::setfill('0') << std::setw(3) << checksum << '\x01';
        
        return finalMsg.str();
    }

    std::string OutboundMessageBuilder::buildLogon(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, int heartbeatInterval) {
        std::ostringstream body;
        // 98=EncryptMethod(0=None), 108=HeartBtInt
        body << "98=0\x01" << "108=" << heartbeatInterval << '\x01';
        return buildMessage(senderCompId, targetCompId, msgSeqNum, "A", body.str());
    }

    std::string OutboundMessageBuilder::buildLogout(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, const std::string& text) {
        std::ostringstream body;
        if (!text.empty()) {
            body << "58=" << text << '\x01'; // 58=Text
        }
        return buildMessage(senderCompId, targetCompId, msgSeqNum, "5", body.str());
    }

    std::string OutboundMessageBuilder::buildHeartbeat(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, const std::string& testReqId) {
        std::ostringstream body;
        if (!testReqId.empty()) {
            body << "112=" << testReqId << '\x01'; // 112=TestReqID
        }
        return buildMessage(senderCompId, targetCompId, msgSeqNum, "0", body.str());
    }

    std::string OutboundMessageBuilder::buildResendRequest(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, uint32_t beginSeqNo, uint32_t endSeqNo) {
        std::ostringstream body;
        // 7=BeginSeqNo, 16=EndSeqNo
        body << "7=" << beginSeqNo << '\x01' << "16=" << endSeqNo << '\x01';
        return buildMessage(senderCompId, targetCompId, msgSeqNum, "2", body.str());
    }

} // namespace fix
