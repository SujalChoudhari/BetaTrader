#include "trading_core/TradeIDGenerator.h"
#include <future>

namespace trading_core {

    TradeIDGenerator::TradeIDGenerator(data::DatabaseWorker* dbWorker)
        : mCurrentId(0), mDatabaseWorker(dbWorker) {
        loadInitialState();
    }

    common::TradeID TradeIDGenerator::nextId() {
        return ++mCurrentId;
    }

    void TradeIDGenerator::loadInitialState() {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        mDatabaseWorker->enqueue([this, promise](SQLite::Database& db) {
            try {
                SQLite::Statement query(db, "SELECT MAX(trade_id) FROM trades;");
                if (query.executeStep()) {
                    mCurrentId = query.getColumn(0).getInt64();
                }
            } catch (const std::exception& e) {
                // Table might not exist or be empty, which is fine.
                // In that case, the ID will start from 0.
            }
            promise->set_value();
        });

        future.wait();
    }

}
