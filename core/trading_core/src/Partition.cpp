#include "trading_core/Partition.h"
#include "logging/Logger.h"

namespace trading_core {
    Partition::Partition(common::Instrument symbol, data::DatabaseWorker* databaseWorker, TradeIDGenerator* tradeIDGenerator)
        : mCommandQueue(262144),
          mSymbol(symbol),
          mDatabaseWorker(databaseWorker),
          mTradeIDGenerator(tradeIDGenerator) {
        mTradeRepository = std::make_unique<data::TradeRepository>(mDatabaseWorker);
        mOrderManager = std::make_unique<OrderManager>();
        mOrderBook = std::make_unique<OrderBook>();
        mMatcher = std::make_unique<Matcher>(mTradeIDGenerator);
        mRiskManager = std::make_unique<RiskManager>(mTradeRepository.get());
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
        mWorker = std::make_unique<WorkerThread>(mCommandQueue, *mOrderManager, *mOrderBook, *mMatcher, *mRiskManager, mTradeIDGenerator, mDatabaseWorker);
        mWorker->start();
        LOG_INFO("Worker thread started for partition {}.", common::to_string(mSymbol));
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
            LOG_ERROR("ETRADE1", "Partition {} command queue full.", common::to_string(mSymbol));
        }
    }

    size_t Partition::getQueueSize() const {
        return mCommandQueue.size();
    }

    common::Symbol Partition::getSymbol() const {
        return mSymbol;
    }

    const data::DatabaseWorker* Partition::getDatabaseWorker() const {
        return mDatabaseWorker;
    }

    const TradeIDGenerator* Partition::getTradeIDGenerator() const {
        return mTradeIDGenerator;
    }

    const OrderManager* Partition::getOrderManager() const {
        return mOrderManager.get();
    }

    const OrderBook* Partition::getOrderBook() const {
        return mOrderBook.get();
    }

    const Matcher* Partition::getMatcher() const {
        return mMatcher.get();
    }

    const RiskManager* Partition::getRiskManager() const {
        return mRiskManager.get();
    }

    const WorkerThread* Partition::getWorker() const {
        return mWorker.get();
    }
}