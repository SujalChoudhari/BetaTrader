/**
 * @file WorkerThread.h
 * @brief Worker thread that consumes command queue and executes trading logic.
 *
 * The WorkerThread reads commands from a single-producer single-consumer queue,
 * delegates them to the appropriate handlers (new/modify/cancel), and applies
 * matching and persistence operations.
 */

#pragma once
#include <exchange_routing/CancelOrder.h>
#include <exchange_routing/Command.h>
#include <exchange_publishers/ExecutionPublisher.h>
#include <exchange_matching/Matcher.h>
#include <exchange_routing/ModifyOrder.h>
#include <exchange_routing/NewOrder.h>
#include <exchange_matching/OrderBook.h>
#include <exchange_state/OrderManager.h>
#include <exchange_risk/RiskManager.h>
#include "data/OrderRepository.h"
#include "rigtorp/SPSCQueue.h"
#include <memory>
#include <stop_token>
#include <thread>

namespace trading_core {
    /**
     * @class WorkerThread
     * @brief Background worker which processes batched commands.
     */
    class WorkerThread {
    public:
        WorkerThread(rigtorp::SPSCQueue<std::unique_ptr<Command>>& commandQueue,
                     OrderManager& orderManager, OrderBook& orderBook,
                     Matcher& matcher, RiskManager& riskManager,
                     data::OrderRepository& orderRepository,
                     TradeIDGenerator* tradeIDGenerator,
                     data::DatabaseWorker* databaseWorker);

        ~WorkerThread();

        void start();

        void stop();

        // Public method for deterministic testing
        void processNextCommand();

    private:
        void runLoop(std::stop_token stopToken);

        void processNewOrder(NewOrder& cmd) const;

        void processCancelOrder(const CancelOrder& cmd) const;

        void processModifyOrder(const ModifyOrder& cmd) const;

        void processBatch(std::unique_ptr<Command>* commands, size_t count);

        std::jthread mThread;
        rigtorp::SPSCQueue<std::unique_ptr<Command>>& mCommandQueue;

        OrderManager& mOrderManager;
        OrderBook& mOrderBook;
        Matcher& mMatcher;
        RiskManager& mRiskManager;
        data::OrderRepository& mOrderRepository;

        TradeIDGenerator* mTradeIDGenerator;
        data::DatabaseWorker* mDatabaseWorker;

        static constexpr size_t BATCH_SIZE = 64;
        std::unique_ptr<Command> mCommandBatch[BATCH_SIZE];
    };
} // namespace trading_core
