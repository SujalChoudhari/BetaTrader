#pragma once

#include <string>
#include <cstdint>

namespace fix {

    class OutboundMessageBuilder {
    public:
        // Helper to format timestamps consistently for FIX 4.4 (YYYYMMDD-HH:MM:SS.sss)
        static std::string generateTimestamp();

        // Helper to construct the complete FIX string (calculates BodyLength and Checksum)
        static std::string buildMessage(const std::string& senderCompId, 
                                        const std::string& targetCompId, 
                                        uint32_t msgSeqNum, 
                                        const std::string& msgType, 
                                        const std::string& bodyStr);

        // Pre-canned builders for session messages
        static std::string buildLogon(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, int heartbeatInterval);
        static std::string buildLogout(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, const std::string& text = "");
        static std::string buildHeartbeat(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, const std::string& testReqId = "");
        static std::string buildResendRequest(const std::string& senderCompId, const std::string& targetCompId, uint32_t msgSeqNum, uint32_t beginSeqNo, uint32_t endSeqNo);
    };

} // namespace fix
