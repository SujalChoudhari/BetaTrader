#include "trading-core/TradeIDGenerator.h"
#include "common/Types.h"

namespace trading_core {
    TradeIDGenerator::TradeIDGenerator(const data::DatabaseWorkerPtr &dbWorker) : repository(
        data::TradeIDRepository(dbWorker)) {
        loadState();
    }

    common::TradeID TradeIDGenerator::getId() {
        if (mCurrentId == 0)
            loadState();
        return mCurrentId;
    }

    common::TradeID TradeIDGenerator::nextId() {
        std::lock_guard lock(mMutex);
        ++mCurrentId;
        saveState();
        return mCurrentId;
    }

    void TradeIDGenerator::saveState() {
        repository.setCurrentTradeID(mCurrentId);
    }

    void TradeIDGenerator::loadState() {
        mCurrentId = repository.getCurrentTradeID();
    }
}
