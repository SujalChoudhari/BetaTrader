/**
 * @file Partition.h
 * @brief Represents a partition (shard) of the trading engine for a single
 * symbol.
 *
 * Partitions are independent units that contain an order book, matcher, risk
 * manager, and a worker thread. They are suitable for parallel processing of
 * different instruments.
 */

#pragma once
#include "Command.h"
#include "Matcher.h"
#include "OrderBook.h"
#include "OrderManager.h"
#include "RiskManager.h"
#include "WorkerThread.h"
#include "data/OrderRepository.h"
#include "data/TradeRepository.h"
#include "rigtorp/SPSCQueue.h"
#include <atomic>
#include <future>
#include <memory>

namespace trading_core {
    /**
     * @class Partition
     * @brief Encapsulates all components needed to process commands for a
     * symbol.
     */
    class Partition {
    public:
        Partition(common::Instrument symbol,
                  data::DatabaseWorker* databaseWorker,
                  TradeIDGenerator* tradeIDGenerator);

        Partition(common::Instrument symbol,
                  data::DatabaseWorker* databaseWorker,
                  TradeIDGenerator* tradeIDGenerator,
                  std::unique_ptr<data::TradeRepository> tradeRepository,
                  std::unique_ptr<data::OrderRepository> orderRepository,
                  std::unique_ptr<OrderManager> orderManager,
                  std::unique_ptr<OrderBook> orderBook,
                  std::unique_ptr<Matcher> matcher,
                  std::unique_ptr<RiskManager> riskManager);

        ~Partition();

        void start();

        void stop();

        void stopAcceptingCommands();

        void enqueue(std::unique_ptr<Command> command);

        size_t getQueueSize() const;

        void waitReady();

        [[nodiscard]] common::Symbol getSymbol() const;

        [[nodiscard]] const data::DatabaseWorker* getDatabaseWorker() const;

        [[nodiscard]] const TradeIDGenerator* getTradeIDGenerator() const;

        [[nodiscard]] const OrderManager* getOrderManager() const;

        [[nodiscard]] const OrderBook* getOrderBook() const;

        [[nodiscard]] const Matcher* getMatcher() const;

        [[nodiscard]] const RiskManager* getRiskManager() const;

        [[nodiscard]] const data::OrderRepository* getOrderRepository() const;

        [[nodiscard]] const WorkerThread* getWorker() const;

    private:
        void init();

        rigtorp::SPSCQueue<std::unique_ptr<Command>> mCommandQueue;
        std::atomic<bool> mAcceptingCommands{true};
        common::Symbol mSymbol;
        data::DatabaseWorker* mDatabaseWorker;
        TradeIDGenerator* mTradeIDGenerator;

        std::unique_ptr<data::TradeRepository> mTradeRepository;
        std::unique_ptr<data::OrderRepository> mOrderRepository;
        std::unique_ptr<OrderManager> mOrderManager;
        std::unique_ptr<OrderBook> mOrderBook;
        std::unique_ptr<Matcher> mMatcher;
        std::unique_ptr<RiskManager> mRiskManager;
        std::unique_ptr<WorkerThread> mWorker;
        std::future<void> mReadyFuture;
    };
} // namespace trading_core
