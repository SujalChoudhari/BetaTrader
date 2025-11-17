//
// Created by sujal on 25-10-2025.
//

#include "trading_core/OrderIDGenerator.h"
#include <future>

namespace trading_core {

    OrderIDGenerator::OrderIDGenerator(data::DatabaseWorker* dbWorker)
        : mCurrentId(0), mDatabaseWorker(dbWorker)
    {
        loadInitialState();
    }

    common::OrderID OrderIDGenerator::nextId()
    {
        return ++mCurrentId;
    }

    void OrderIDGenerator::loadInitialState()
    {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        mDatabaseWorker->enqueue([this, promise](SQLite::Database& db) {
            try {
                SQLite::Statement query(db,
                                        "SELECT MAX(order_id) FROM orders;");
                if (query.executeStep()) {
                    mCurrentId = query.getColumn(0).getInt64();
                }
            }
            catch (const std::exception& e) {
                // Table might not exist or be empty, which is fine.
                // In that case, the ID will start from 0.
            }
            promise->set_value();
        });

        future.wait();
    }

} // namespace trading_core
