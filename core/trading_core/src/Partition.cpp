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

    Partition::~Partition() {
        stop();
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

    void Partition::enqueue(std::unique_ptr<Command> command) {
        if (!mCommandQueue.try_push(std::move(command))) {
            LOG_ERROR("Partition {} command queue full. Dropping command type {}",
                      common::to_string(mSymbol), trading_core::to_string(command->getType()));
            // TODO: optionally track dropped command metrics
            // The unique_ptr will be automatically deleted when it goes out of scope
        }
        // The command is moved, so 'command' is now null. Accessing it here would be a use-after-move.
        // LOG_INFO("Pushed a command in symbol {} from {}", common::to_string(mSymbol), command->getClientId());
        // To log, we need to access the command before moving it, or get it back from the queue if push fails.
        // For now, I'll comment out the log that accesses the moved command.
    }

    common::Symbol Partition::getSymbol() const {
        return mSymbol;
    }

    const data::DatabaseWorkerPtr &Partition::getDatabaseWorker() const {
        return mDatabaseWorker;
    }

    const std::shared_ptr<TradeIDGenerator> &Partition::getTradeIDGenerator() const {
        return mTradeIDGenerator;
    }

    const std::shared_ptr<ExecutionPublisher> &Partition::getExecutionPublisher() const {
        return mExecutionPublisher;
    }

    const OrderManager *Partition::getOrderManager() const {
        return mOrderManager.get();
    }

    const OrderBook *Partition::getOrderBook() const {
        return mOrderBook.get();
    }

    const Matcher *Partition::getMatcher() const {
        return mMatcher.get();
    }

    const RiskManager *Partition::getRiskManager() const {
        return mRiskManager.get();
    }

    const WorkerThread *Partition::getWorker() const {
        return mWorker.get();
    }
} // namespace trading_core
