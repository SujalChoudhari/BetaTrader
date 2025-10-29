//
// Created by sujal on 29-10-2025.
//

#include "trading_core/Partition.h"
#include "trading_core/CommandType.h"
#include "logging/Logger.h"
#include "trading_core/WorkerThread.h"

namespace trading_core {
    Partition::Partition(
        common::Instrument symbol,
        data::DatabaseWorkerPtr databaseWorker,
        std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
        std::shared_ptr<ExecutionPublisher> executionPublisher
    )
        : mCommandQueue(4096)
          , mSymbol(symbol)
          , mDatabaseWorker(std::move(databaseWorker))
          , mTradeIDGenerator(std::move(tradeIDGenerator))
          , mExecutionPublisher(std::move(executionPublisher)) {
        mOrderManager = std::make_unique<OrderManager>();
        mOrderBook = std::make_unique<OrderBook>();
        mMatcher = std::make_unique<Matcher>(mDatabaseWorker);
        mRiskManager = std::make_unique<RiskManager>(mDatabaseWorker);
    }

    void Partition::start() {
        if (mWorker) {
            LOG_INFO("Partition for symbol {} already running", common::to_string(mSymbol));
            return;
        }

        LOG_INFO("Starting partition for symbol {}", common::to_string(mSymbol));

        mWorker = std::make_unique<WorkerThread>(
            mCommandQueue,
            *mOrderManager,
            *mOrderBook,
            *mMatcher,
            *mRiskManager,
            mExecutionPublisher,
            mTradeIDGenerator,
            mDatabaseWorker
        );

        mWorker->start();
    }

    void Partition::stop() {
        if (mWorker) {
            LOG_INFO("Stopping partition for symbol {}", common::to_string(mSymbol));
            mWorker->stop();
            mWorker.reset();
        }
    }

    void Partition::enqueue(Command *command) {
        if (!mCommandQueue.try_push(command)) {
            LOG_ERROR("Partition {} command queue full. Dropping command type {}",
                      common::to_string(mSymbol), trading_core::to_string(command->getType()));
            // TODO: optionally track dropped command metrics
            delete command; // prevent leak
        }
    }
} // namespace trading_core
