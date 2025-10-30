//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <mutex>
#include <atomic>

#include "common/Types.h"
#include "data/TradeIDRepository.h"

namespace trading_core {
    class TradeIDGenerator {
    public:
        explicit TradeIDGenerator(const data::DatabaseWorkerPtr &dbWorker);

        common::TradeID getId();

        common::TradeID nextId();

        void saveState();

        void loadState();

    private:
        std::atomic<common::TradeID> mCurrentId;
        std::mutex mMutex;
        data::TradeIDRepository repository;
    };
}
