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
#include <memory>

namespace trading_core {
    /**
     * @class Partition
     * @brief Represents a single trading partition responsible for processing commands for a specific instrument.
     *
     * Each partition manages its own order book, order manager, matcher, risk manager,
     * and a worker thread to process incoming commands asynchronously.
     */
    class Partition {
    public:
        /**
         * @brief Constructs a new Partition object.
         * @param symbol The financial instrument symbol this partition handles.
         * @param databaseWorker A shared pointer to the database worker for persistence.
         * @param tradeIDGenerator A shared pointer to the trade ID generator.
         * @param executionPublisher A shared pointer to the execution publisher.
         */
        Partition(common::Instrument symbol,
                  data::DatabaseWorkerPtr databaseWorker,
                  std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
                  std::shared_ptr<ExecutionPublisher> executionPublisher);

        /**
         * @brief Destroys the Partition object, stopping the worker thread and cleaning up resources.
         */
        ~Partition();

        /**
         * @brief Starts the worker thread for this partition.
         */
        void start();

        /**
         * @brief Stops the worker thread for this partition.
         */
        void stop();

        /**
         * @brief Enqueues a command for asynchronous processing by the worker thread.
         * @param command A unique pointer to the command to be enqueued.
         */
        void enqueue(std::unique_ptr<Command> command);

    public:
        /** @brief Gets the financial instrument symbol handled by this partition. */
        [[nodiscard]] common::Symbol getSymbol() const;

        /** @brief Gets a constant reference to the database worker. */
        [[nodiscard]] const data::DatabaseWorkerPtr &getDatabaseWorker() const;

        /** @brief Gets a constant reference to the trade ID generator. */
        [[nodiscard]] const std::shared_ptr<TradeIDGenerator> &getTradeIDGenerator() const;

        /** @brief Gets a constant reference to the execution publisher. */
        [[nodiscard]] const std::shared_ptr<ExecutionPublisher> &getExecutionPublisher() const;

        /** @brief Gets a constant pointer to the OrderManager. */
        [[nodiscard]] const OrderManager *getOrderManager() const;

        /** @brief Gets a constant pointer to the OrderBook. */
        [[nodiscard]] const OrderBook *getOrderBook() const;

        /** @brief Gets a constant pointer to the Matcher. */
        [[nodiscard]] const Matcher *getMatcher() const;

        /** @brief Gets a constant pointer to the RiskManager. */
        [[nodiscard]] const RiskManager *getRiskManager() const;

        /** @brief Gets a constant pointer to the WorkerThread. */
        [[nodiscard]] const WorkerThread *getWorker() const;

    private:
        rigtorp::SPSCQueue<std::unique_ptr<Command>> mCommandQueue; ///< The single-producer, single-consumer queue for commands.

        common::Symbol mSymbol;                                     ///< The financial instrument symbol this partition handles.
        data::DatabaseWorkerPtr mDatabaseWorker;                    ///< Shared pointer to the database worker.
        std::shared_ptr<TradeIDGenerator> mTradeIDGenerator;        ///< Shared pointer to the trade ID generator.

        std::unique_ptr<OrderManager> mOrderManager;                ///< Unique pointer to the OrderManager.
        std::unique_ptr<OrderBook> mOrderBook;                     ///< Unique pointer to the OrderBook.
        std::unique_ptr<Matcher> mMatcher;                         ///< Unique pointer to the Matcher.
        std::unique_ptr<RiskManager> mRiskManager;                 ///< Unique pointer to the RiskManager.

        std::unique_ptr<WorkerThread> mWorker;                      ///< Unique pointer to the WorkerThread.
        std::shared_ptr<ExecutionPublisher> mExecutionPublisher;    ///< Shared pointer to the ExecutionPublisher.
    };
}
