#pragma once

#include "data/SequenceRepository.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "common_fix/SessionState.h"

namespace fix {

    class FixSessionManager {
    public:
        // Optional repository for sequence persistence
        FixSessionManager(::data::SequenceRepository* seqRepo = nullptr);

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
        ::data::SequenceRepository* mSeqRepo;
    };

} // namespace fix
