//
// Created by sujal on 29-10-2025.
//

#pragma once
#include "CancelOrder.h"
#include "Command.h"
#include "ExecutionPublisher.h"
#include "Matcher.h"
#include "ModifyOrder.h"
#include "NewOrder.h"
#include "OrderBook.h"
#include "OrderManager.h"
#include "RiskManager.h"
#include "rigtorp/SPSCQueue.h"


namespace trading_core {
    class WorkerThread {
    public:
        WorkerThread(
            rigtorp::SPSCQueue<Command *> &commandQueue,
            OrderManager &orderManager,
            OrderBook &orderBook,
            Matcher &matcher,
            RiskManager &riskManager,
            std::shared_ptr<ExecutionPublisher> executionPublisher,
            std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
            data::DatabaseWorkerPtr databaseWorker
        );

        ~WorkerThread();

        // Start the worker thread
        void start();

        // Stop the worker thread (blocks until thread exits)
        void stop();

        // Main event loop (runs in separate thread)
        void runLoop();

    private:
        // Process a batch of commands
        void processBatch(Command **commands, size_t count) const;

        // Process individual command types
        void processNewOrder(const NewOrder *cmd) const;

        void processCancelOrder(const CancelOrder *cmd) const;

        void processModifyOrder(const ModifyOrder *cmd) const;

        // Thread control
        std::thread mThread;
        std::atomic<bool> mRunning;

        // Reference to partition's command queue
        rigtorp::SPSCQueue<Command *> &mCommandQueue;

        // References to partition-owned components
        OrderManager &mOrderManager;
        OrderBook &mOrderBook;
        Matcher &mMatcher;
        RiskManager &mRiskManager;

        // Shared components
        std::shared_ptr<ExecutionPublisher> mExecutionPublisher;
        std::shared_ptr<TradeIDGenerator> mTradeIDGenerator;
        data::DatabaseWorkerPtr mDatabaseWorker;

        // Batch processing buffer
        static constexpr size_t BATCH_SIZE = 64;
        Command *mCommandBatch[BATCH_SIZE];
    };
}
