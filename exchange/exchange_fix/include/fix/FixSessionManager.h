#pragma once

#include "data/SequenceRepository.h"
#include <cstdint>
#include <mutex>
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
        bool validateSequence(uint32_t sessionId, uint32_t incomingSeqNum, bool isLogon = false);

        // Handle a Logout request (marks session offline, preserves mapping for ack).
        void handleLogout(uint32_t sessionId);

        // Cleans up the connection-to-CompID mapping after the logout ack has been sent.
        void cleanupConnection(uint32_t sessionId);

        // Get current session state (returns nullptr if session not found)
        SessionState* getSessionState(uint32_t sessionId);

        /**
         * @brief Increments and persists the outbound sequence number.
         * @return The sequence number to use for the message.
         */
        uint32_t useNextOutboundSequence(uint32_t sessionId);

        /**
         * @brief Increments the outbound sequence number without returning it.
         */
        void incrementOutboundSequence(uint32_t sessionId);


        /// Returns a thread-safe snapshot of all session states.
        std::unordered_map<std::string, SessionState> getAllSessionStates() const;

        /// Returns a reference to the internal mutex for external synchronization.
        std::mutex& getMutex() { return mMutex; }


    private:
        mutable std::mutex mMutex;
        std::unordered_map<std::string, bool> mValidClients;
        // States keyed by SenderCompID
        std::unordered_map<std::string, SessionState> mSessionStates;
        // Mapping of active connection ID to its authenticated CompID
        std::unordered_map<uint32_t, std::string> mConnectionToCompId;
        ::data::SequenceRepository* mSeqRepo;
    };


} // namespace fix
