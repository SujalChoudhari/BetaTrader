/**
 * @file SeqNumStore.h
 * @brief Manages persistence of FIX sequence numbers for the client.
 */

#pragma once

#include <cstdint>
#include <string>
#include <mutex>

namespace fix_client {

    /**
     * @class SeqNumStore
     * @brief A thread-safe, file-backed repository for FIX Sequence Numbers.
     * 
     * To prevent `ResendRequest` loops on startup, the client must remember 
     * its last known sequence numbers. This class saves them to disk.
     */
    class SeqNumStore {
    public:
        /**
         * @brief Constructs a new SeqNumStore.
         * @param senderCompId The component ID, used to generate the filename.
         * @param directory The directory to save the sequence files (defaults to local dir).
         */
        SeqNumStore(const std::string& senderCompId, const std::string& directory = "seq_store");

        /**
         * @brief Gets the next expected incoming sequence number.
         */
        uint32_t getNextTargetSeqNum() const;

        /**
         * @brief Gets the next expected outgoing sequence number.
         */
        uint32_t getNextSenderSeqNum() const;

        /**
         * @brief Sets both sequence numbers and persists them to disk.
         * @param inSeq The incoming sequence number (TargetSeqNum, from server).
         * @param outSeq The outgoing sequence number (SenderSeqNum, from client).
         */
        void setSeqNums(uint32_t inSeq, uint32_t outSeq);

        /**
         * @brief Resets the sequence numbers to 1 (e.g., on a new day or clean Logon).
         */
        void reset();

    private:
        void load();
        void save() const;
        void saveInternal() const;

        std::string mFilePath;
        mutable std::mutex mMutex; // Mutable to lock during const getters
        
        uint32_t mInSeqNum = 1;
        uint32_t mOutSeqNum = 1;
    };

} // namespace fix_client
