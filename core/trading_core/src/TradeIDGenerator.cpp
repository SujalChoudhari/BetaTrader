#include "trading_core/TradeIDGenerator.h"
#include "common/Types.h"
#include "logging/Logger.h"

namespace trading_core {
    TradeIDGenerator::TradeIDGenerator(const data::DatabaseWorkerPtr &dbWorker) : repository(
        data::TradeIDRepository(dbWorker)) {
        LOG_INFO("TradeIDGenerator instance created at {}", (void*)this);
        loadState();
    }

    TradeIDGenerator::~TradeIDGenerator() {
        LOG_INFO("Saving final TradeID state from generator at {}...", (void*)this);
        saveState();
    }

    common::TradeID TradeIDGenerator::getId() {
        return mCurrentId.load(std::memory_order_relaxed);
    }

    common::TradeID TradeIDGenerator::nextId() {
        // Only increment in memory for performance; state is saved on destruction.
        return ++mCurrentId;
    }

    void TradeIDGenerator::saveState() {
        std::lock_guard lock(mMutex);
        repository.setCurrentTradeID(mCurrentId.load(std::memory_order_relaxed));
    }

    void TradeIDGenerator::loadState() {
        std::lock_guard lock(mMutex);
        mCurrentId = repository.getCurrentTradeID();
    }
}
