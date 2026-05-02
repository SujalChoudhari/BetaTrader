#include "fix/FixSessionManager.h"
#include "logging/Logger.h"
#include "data/SequenceRepository.h"

namespace fix {

    FixSessionManager::FixSessionManager(data::SequenceRepository* seqRepo) : mSeqRepo(seqRepo) {
    }

    void FixSessionManager::loadConfig(const std::vector<std::string>& validClients) {
        std::lock_guard<std::mutex> lock(mMutex);
        mValidClients.clear();
        for (const auto& client : validClients) {
            mValidClients[client] = true;
        }
        LOG_INFO("FixSessionManager: Loaded {} valid clients for authentication.", validClients.size());
    }

    bool FixSessionManager::authenticate(uint32_t sessionId, const std::string& senderCompId) {
        std::lock_guard<std::mutex> lock(mMutex);
        
        // Auto-trust simulator agents (IDs starting with SIM_)
        bool isSimulator = (senderCompId.rfind("SIM_", 0) == 0);

        if (!isSimulator && mValidClients.find(senderCompId) == mValidClients.end()) {
            LOG_WARN("Session {} failed authentication: Unknown SenderCompID '{}'", sessionId, senderCompId);
            return false;
        }

        // Link connection ID to CompID
        mConnectionToCompId[sessionId] = senderCompId;

        // Retrieve or initialize persistent state for this CompID
        auto it = mSessionStates.find(senderCompId);
        if (it == mSessionStates.end()) {
            SessionState state;
            state.senderCompId = senderCompId;
            
            if (mSeqRepo) {
                auto seqs = mSeqRepo->getSequenceNumbers(senderCompId);
                state.inSeqNum = std::get<0>(seqs);
                state.outSeqNum = std::get<1>(seqs);
                LOG_INFO("SessionManager: Loaded persistent seqs for {} (IN: {}, OUT: {})", senderCompId, state.inSeqNum, state.outSeqNum);
            } else {
                state.inSeqNum = 0; 
                state.outSeqNum = 1; 
            }
            mSessionStates[senderCompId] = state;
            it = mSessionStates.find(senderCompId);
        }

        it->second.isLoggedOn = true;
        LOG_INFO("Session {} successfully authenticated as {}", sessionId, senderCompId);
        return true;
    }

    bool FixSessionManager::validateSequence(uint32_t sessionId, uint32_t incomingSeqNum, bool isLogon) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto connIt = mConnectionToCompId.find(sessionId);
        if (connIt == mConnectionToCompId.end()) {
            LOG_ERROR("Cannot validate sequence for unknown session ID {}", sessionId);
            return false;
        }

        const std::string& compId = connIt->second;
        auto stateIt = mSessionStates.find(compId);
        if (stateIt == mSessionStates.end() || !stateIt->second.isLoggedOn) {
            LOG_ERROR("Cannot validate sequence for unauthenticated client {}", compId);
            return false;
        }

        SessionState& state = stateIt->second;
        uint32_t expectedSeqNum = state.inSeqNum + 1;

        if (incomingSeqNum == expectedSeqNum) {
            state.inSeqNum = incomingSeqNum;
            if (mSeqRepo) {
                mSeqRepo->updateSequenceNumbers(compId, state.inSeqNum, state.outSeqNum);
            }
            return true;
        } else if (incomingSeqNum > expectedSeqNum) {
            if (isLogon) {
                LOG_INFO("Sequence sync on Logon for {}: Synced from {} to {}", compId, expectedSeqNum, incomingSeqNum);
                state.inSeqNum = incomingSeqNum;
                if (mSeqRepo) {
                    mSeqRepo->updateSequenceNumbers(compId, state.inSeqNum, state.outSeqNum);
                }
                return true;
            }
            LOG_WARN("Sequence gap detected for {}. Expected {}, got {}", compId, expectedSeqNum, incomingSeqNum);
            return false;
        } else {
            LOG_ERROR("Fatal sequence error for {}. Expected {}, got {}", compId, expectedSeqNum, incomingSeqNum);
            return false;
        }
    }

    void FixSessionManager::handleLogout(uint32_t sessionId) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto connIt = mConnectionToCompId.find(sessionId);
        if (connIt != mConnectionToCompId.end()) {
            const std::string& compId = connIt->second;
            auto stateIt = mSessionStates.find(compId);
            if (stateIt != mSessionStates.end()) {
                stateIt->second.isLoggedOn = false;
                LOG_INFO("Client {} logged out from session {}.", compId, sessionId);
            }
            // NOTE: Do NOT erase mConnectionToCompId here.
            // The mapping is still needed by useNextOutboundSequence() to
            // build the logout acknowledgment. Call cleanupConnection()
            // after the ack has been sent.
        }
    }

    void FixSessionManager::cleanupConnection(uint32_t sessionId) {
        std::lock_guard<std::mutex> lock(mMutex);
        mConnectionToCompId.erase(sessionId);
    }

    SessionState* FixSessionManager::getSessionState(uint32_t sessionId) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto connIt = mConnectionToCompId.find(sessionId);
        if (connIt != mConnectionToCompId.end()) {
            auto stateIt = mSessionStates.find(connIt->second);
            if (stateIt != mSessionStates.end()) {
                return &(stateIt->second);
            }
        }
        return nullptr;
    }

    uint32_t FixSessionManager::useNextOutboundSequence(uint32_t sessionId) {
        std::lock_guard<std::mutex> lock(mMutex);
        // Inline the lookup instead of calling getSessionState (which also locks)
        auto connIt = mConnectionToCompId.find(sessionId);
        if (connIt == mConnectionToCompId.end()) return 1;
        auto stateIt = mSessionStates.find(connIt->second);
        if (stateIt == mSessionStates.end()) return 1;
        SessionState* state = &(stateIt->second);

        uint32_t current = state->outSeqNum;
        state->outSeqNum++;
        
        if (mSeqRepo) {
            mSeqRepo->updateSequenceNumbers(state->senderCompId, state->inSeqNum, state->outSeqNum);
        }
        
        return current;
    }

    void FixSessionManager::incrementOutboundSequence(uint32_t sessionId) {
        useNextOutboundSequence(sessionId);
    }

    std::unordered_map<std::string, SessionState> FixSessionManager::getAllSessionStates() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mSessionStates; // Returns a copy — safe for cross-thread use
    }

} // namespace fix

