#include "trading_core/Partition.h"
#include "trading_core/CommandType.h"
#include "logging/Logger.h"
#include "trading_core/WorkerThread.h"
#include "trading_core/TradingCoreRunbookDefinations.h"

namespace trading_core {
    Partition::Partition(
        common::Instrument symbol,
        data::DatabaseWorkerPtr databaseWorker,
        std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
        std::shared_ptr<ExecutionPublisher> executionPublisher
    )
        : mCommandQueue(262144) // Increased queue size for high-throughput scenarios
          , mSymbol(symbol)
          , mDatabaseWorker(std::move(databaseWorker))
          , mTradeIDGenerator(std::move(tradeIDGenerator))
          , mExecutionPublisher(std::move(executionPublisher)) {
        mOrderManager = std::make_unique<OrderManager>();
        mOrderBook = std::make_unique<OrderBook>();
        // Pass the shared TradeIDGenerator to the Matcher
        mMatcher = std::make_unique<Matcher>(mTradeIDGenerator);
        mRiskManager = std::make_unique<RiskManager>(mDatabaseWorker);
        LOG_INFO("Partition for symbol {} initialized.", common::to_string(mSymbol));
    }

    Partition::~Partition() {
        stop();
        LOG_INFO("Partition for symbol {} destroyed.", common::to_string(mSymbol));
    }

    void Partition::start() {
        if (mWorker) {
            LOG_WARN("Partition for symbol {} already running. Skipping start.", common::to_string(mSymbol));
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
        LOG_INFO("Worker thread started for partition {}.", common::to_string(mSymbol));
    }

    void Partition::stop() {
        if (mWorker) {
            LOG_INFO("Stopping partition for symbol {}", common::to_string(mSymbol));
            mWorker->stop();
            mWorker.reset();
            LOG_INFO("Worker thread stopped and reset for partition {}.", common::to_string(mSymbol));
        } else {
            LOG_WARN("Partition for symbol {} is not running. Skipping stop.", common::to_string(mSymbol));
        }
    }

    void Partition::enqueue(std::unique_ptr<Command> command) {
        if (!command) {
            LOG_ERROR(errors::ETRADE4, "Attempted to enqueue a null command in partition {}.", common::to_string(mSymbol));
            return;
        }

        if (!mCommandQueue.try_push(std::move(command))) {
            LOG_ERROR(errors::ETRADE1, "Partition {} command queue full. Dropping command type {}.",
                      common::to_string(mSymbol), trading_core::to_string(command->getType()));
        } else {
            LOG_DEBUG("Enqueued command type {} in partition {}.", trading_core::to_string(command->getType()), common::to_string(mSymbol));
        }
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
