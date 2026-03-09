#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <vector>

namespace fix {

    struct SessionState {
        bool isLoggedOn = false;
        uint32_t inSeqNum = 0;
        uint32_t outSeqNum = 0;
    };

    class FixSessionManager {
    public:
        FixSessionManager();

        // Load valid clients provided by the database config
        void loadConfig(const std::vector<std::string>& validClients);

        // Attempts to authenticate a Logon request.
        // Returns true if SenderCompID is valid.
        bool authenticate(uint32_t sessionId, const std::string& senderCompId);

        // Validates an incoming sequence number.
        // Returns true if strictly next expected, false if gap or too low.
        bool validateSequence(uint32_t sessionId, uint32_t incomingSeqNum);

        // Handle a Logout request.
        void handleLogout(uint32_t sessionId);

        // Get current session state (returns nullptr if session not found)
        SessionState* getSessionState(uint32_t sessionId);

    private:
        std::unordered_map<std::string, bool> mValidClients;
        std::unordered_map<uint32_t, SessionState> mSessionStates;
    };

} // namespace fix
