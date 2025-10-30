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
#include <memory>


namespace trading_core {
    /**
     * @class WorkerThread
     * @brief A dedicated thread for processing commands from a single partition.
     *
     * This class is responsible for dequeuing commands from a partition's command queue
     * and processing them in a tight loop. It interacts with the various components
     * of the trading engine, such as the OrderManager, OrderBook, Matcher, and RiskManager.
     */
    class WorkerThread {
    public:
        /**
         * @brief Constructs a new WorkerThread object.
         * @param commandQueue A reference to the command queue for the partition.
         * @param orderManager A reference to the OrderManager for the partition.
         * @param orderBook A reference to the OrderBook for the partition.
         * @param matcher A reference to the Matcher for the partition.
         * @param riskManager A reference to the RiskManager for the partition.
         * @param executionPublisher A shared pointer to the ExecutionPublisher.
         * @param tradeIDGenerator A shared pointer to the TradeIDGenerator.
         * @param databaseWorker A shared pointer to the DatabaseWorker.
         */
        WorkerThread(
            rigtorp::SPSCQueue<std::unique_ptr<Command>> &commandQueue,
            OrderManager &orderManager,
            OrderBook &orderBook,
            Matcher &matcher,
            RiskManager &riskManager,
            std::shared_ptr<ExecutionPublisher> executionPublisher,
            std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
            data::DatabaseWorkerPtr databaseWorker
        );

        /**
         * @brief Destroys the WorkerThread object, stopping the thread if it is running.
         */
        ~WorkerThread();

        /**
         * @brief Starts the worker thread.
         */
        void start();

        /**
         * @brief Stops the worker thread and blocks until it has exited.
         */
        void stop();

        /**
         * @brief The main event loop for the worker thread.
         */
        void runLoop();

    private:
        /**
         * @brief Processes a batch of commands.
         * @param commands A pointer to an array of commands to be processed.
         * @param count The number of commands in the batch.
         */
        void processBatch(std::unique_ptr<Command> *commands, size_t count);

        /**
         * @brief Processes a NewOrder command.
         * @param cmd The NewOrder command to be processed.
         */
        void processNewOrder(const NewOrder &cmd);

        /**
         * @brief Processes a CancelOrder command.
         * @param cmd The CancelOrder command to be processed.
         */
        void processCancelOrder(const CancelOrder &cmd);

        /**
         * @brief Processes a ModifyOrder command.
         * @param cmd The ModifyOrder command to be processed.
         */
        void processModifyOrder(const ModifyOrder &cmd);

        std::thread mThread;                ///< The worker thread.
        std::atomic<bool> mRunning;         ///< A flag to control the thread's lifecycle.

        rigtorp::SPSCQueue<std::unique_ptr<Command>> &mCommandQueue; ///< A reference to the partition's command queue.

        OrderManager &mOrderManager;        ///< A reference to the partition's OrderManager.
        OrderBook &mOrderBook;              ///< A reference to the partition's OrderBook.
        Matcher &mMatcher;                  ///< A reference to the partition's Matcher.
        RiskManager &mRiskManager;          ///< A reference to the partition's RiskManager.

        std::shared_ptr<ExecutionPublisher> mExecutionPublisher; ///< A shared pointer to the ExecutionPublisher.
        std::shared_ptr<TradeIDGenerator> mTradeIDGenerator;     ///< A shared pointer to the TradeIDGenerator.
        data::DatabaseWorkerPtr mDatabaseWorker;                 ///< A shared pointer to the DatabaseWorker.

        static constexpr size_t BATCH_SIZE = 64;                ///< The maximum number of commands to process in a single batch.
        std::unique_ptr<Command> mCommandBatch[BATCH_SIZE];     ///< A buffer for batch processing commands.
    };
}
