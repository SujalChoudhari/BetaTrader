#include "fix_client/SeqNumStore.h"
#include "logging/Logger.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

namespace fix_client {

    SeqNumStore::SeqNumStore(const std::string& senderCompId, const std::string& directory) {
        fs::create_directories(directory);
        mFilePath = directory + "/" + senderCompId + ".seq";
        load();
    }

    uint32_t SeqNumStore::getNextTargetSeqNum() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mInSeqNum;
    }

    uint32_t SeqNumStore::getNextSenderSeqNum() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mOutSeqNum;
    }

    void SeqNumStore::setSeqNums(uint32_t inSeq, uint32_t outSeq) {
        std::lock_guard<std::mutex> lock(mMutex);
        mInSeqNum = inSeq;
        mOutSeqNum = outSeq;
        save(); // Save while locked
    }

    void SeqNumStore::reset() {
        std::lock_guard<std::mutex> lock(mMutex);
        mInSeqNum = 1;
        mOutSeqNum = 1;
        save();
        LOG_INFO("SeqNumStore reset to 1/1 at {}", mFilePath);
    }

    void SeqNumStore::load() {
        std::lock_guard<std::mutex> lock(mMutex);
        if (fs::exists(mFilePath)) {
            std::ifstream file(mFilePath);
            if (file.is_open()) {
                std::string line;
                if (std::getline(file, line)) {
                    std::istringstream iss(line);
                    if (iss >> mInSeqNum >> mOutSeqNum) {
                        LOG_INFO("SeqNumStore loaded from {}: InSeq={}, OutSeq={}", mFilePath, mInSeqNum, mOutSeqNum);
                        return;
                    }
                }
            }
        }
        
        // If file doesn't exist or is corrupted, start fresh
        mInSeqNum = 1;
        mOutSeqNum = 1;
        save();
        LOG_INFO("SeqNumStore created fresh at {}: InSeq=1, OutSeq=1", mFilePath);
    }

    void SeqNumStore::save() const {
        std::ofstream file(mFilePath, std::ios::trunc);
        if (file.is_open()) {
            file << mInSeqNum << " " << mOutSeqNum << "\n";
        } else {
            LOG_ERROR("SeqNumStore failed to save to {}", mFilePath);
        }
    }

} // namespace fix_client
