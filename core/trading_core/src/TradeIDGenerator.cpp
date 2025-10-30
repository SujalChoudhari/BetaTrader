#include "trading_core/TradeIDGenerator.h"
#include "common/Types.h"

namespace trading_core {
    TradeIDGenerator::TradeIDGenerator(const data::DatabaseWorkerPtr &dbWorker) : repository(
        data::TradeIDRepository(dbWorker)) {
        loadState();
    }

    common::TradeID TradeIDGenerator::getId() {
        return mCurrentId.load(std::memory_order_relaxed);
    }

    common::TradeID TradeIDGenerator::nextId() {
        common::TradeID newId = ++mCurrentId;
        saveState();
        return newId;
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
