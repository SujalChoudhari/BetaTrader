#include "fix/FixSessionManager.h"
#include "logging/Logger.h"

namespace fix {

    FixSessionManager::FixSessionManager() {
    }

    void FixSessionManager::loadConfig(const std::vector<std::string>& validClients) {
        mValidClients.clear();
        for (const auto& client : validClients) {
            mValidClients[client] = true;
        }
        LOG_INFO("FixSessionManager: Loaded {} valid clients for authentication.", validClients.size());
    }

    bool FixSessionManager::authenticate(uint32_t sessionId, const std::string& senderCompId) {
        if (mValidClients.find(senderCompId) != mValidClients.end()) {
            SessionState state;
            state.isLoggedOn = true;
            state.inSeqNum = 0; // Will be incremented when parsing the Logon message
            state.outSeqNum = 1; 
            mSessionStates[sessionId] = state;
            LOG_INFO("Session {} successfully authenticated as {}", sessionId, senderCompId);
            return true;
        }
        
        LOG_WARN("Session {} failed authentication: Unknown SenderCompID '{}'", sessionId, senderCompId);
        return false; 
    }

    bool FixSessionManager::validateSequence(uint32_t sessionId, uint32_t incomingSeqNum) {
        auto it = mSessionStates.find(sessionId);
        if (it == mSessionStates.end() || !it->second.isLoggedOn) {
            LOG_ERROR("Cannot validate sequence for unauthenticated session {}", sessionId);
            return false;
        }

        uint32_t expectedSeqNum = it->second.inSeqNum + 1;

        if (incomingSeqNum == expectedSeqNum) {
            // Perfect case
            it->second.inSeqNum = incomingSeqNum;
            return true;
        } else if (incomingSeqNum > expectedSeqNum) {
            // Gap detected! We need a ResendRequest (MsgType=2)
            LOG_WARN("Sequence gap detected for Session {}. Expected {}, got {}", sessionId, expectedSeqNum, incomingSeqNum);
            return false;
        } else {
            // Sequence too low! Fatal error, must disconnect.
            LOG_ERROR("Fatal sequence error for Session {}. Expected {}, got {}", sessionId, expectedSeqNum, incomingSeqNum);
            return false;
        }
    }

    void FixSessionManager::handleLogout(uint32_t sessionId) {
        auto it = mSessionStates.find(sessionId);
        if (it != mSessionStates.end()) {
            it->second.isLoggedOn = false;
            LOG_INFO("Session {} successfully logged out.", sessionId);
        }
    }

    SessionState* FixSessionManager::getSessionState(uint32_t sessionId) {
        auto it = mSessionStates.find(sessionId);
        if (it != mSessionStates.end()) {
            return &(it->second);
        }
        return nullptr;
    }

} // namespace fix
