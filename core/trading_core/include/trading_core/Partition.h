//
// Created by sujal on 29-10-2025.
//

#pragma once
#include "Command.h"
#include "ExecutionPublisher.h"
#include "Matcher.h"
#include "OrderBook.h"
#include "OrderManager.h"
#include "RiskManager.h"
#include "WorkerThread.h"
#include "rigtorp/SPSCQueue.h"

namespace trading_core {
    class Partition {
    public:
        Partition(common::Instrument symbol,
                  data::DatabaseWorkerPtr databaseWorker,
                  std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
                  std::shared_ptr<ExecutionPublisher> executionPublisher);

        ~Partition();

        void start();

        void stop();

        void enqueue(Command *command);

    public:
        [[nodiscard]] common::Symbol getSymbol() const;

        [[nodiscard]] const data::DatabaseWorkerPtr &getDatabaseWorker() const;

        [[nodiscard]] const std::shared_ptr<TradeIDGenerator> &getTradeIDGenerator() const;

        [[nodiscard]] const std::shared_ptr<ExecutionPublisher> &getExecutionPublisher() const;

        [[nodiscard]] const OrderManager *getOrderManager() const;

        [[nodiscard]] const OrderBook *getOrderBook() const;

        [[nodiscard]] const Matcher *getMatcher() const;

        [[nodiscard]] const RiskManager *getRiskManager() const;

        [[nodiscard]] const WorkerThread *getWorker() const;

    private:
        rigtorp::SPSCQueue<Command *> mCommandQueue;

        common::Symbol mSymbol;
        data::DatabaseWorkerPtr mDatabaseWorker;
        std::shared_ptr<TradeIDGenerator> mTradeIDGenerator;

        std::unique_ptr<OrderManager> mOrderManager;
        std::unique_ptr<OrderBook> mOrderBook;
        std::unique_ptr<Matcher> mMatcher;
        std::unique_ptr<RiskManager> mRiskManager;

        std::unique_ptr<WorkerThread> mWorker;
        std::shared_ptr<ExecutionPublisher> mExecutionPublisher;
    };
}
