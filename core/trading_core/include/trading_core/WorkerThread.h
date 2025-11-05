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
#include "data/OrderRepository.h"
#include "rigtorp/SPSCQueue.h"
#include <memory>
#include <thread>
#include <stop_token>

namespace trading_core {
    class WorkerThread {
    public:
        WorkerThread(
            rigtorp::SPSCQueue<std::unique_ptr<Command> > &commandQueue,
            OrderManager &orderManager,
            OrderBook &orderBook,
            Matcher &matcher,
            RiskManager &riskManager,
            data::OrderRepository &orderRepository,
            TradeIDGenerator *tradeIDGenerator,
            data::DatabaseWorker *databaseWorker
        );

        ~WorkerThread();

        void start();

        void stop();

        // Public method for deterministic testing
        void processNextCommand();

    private:
        void runLoop(std::stop_token stopToken);

        void processNewOrder(NewOrder &cmd) const;

        void processCancelOrder(const CancelOrder &cmd) const;

        void processModifyOrder(const ModifyOrder &cmd) const;

        void processBatch(std::unique_ptr<Command> *commands, size_t count);

        std::jthread mThread;
        rigtorp::SPSCQueue<std::unique_ptr<Command> > &mCommandQueue;

        OrderManager &mOrderManager;
        OrderBook &mOrderBook;
        Matcher &mMatcher;
        RiskManager &mRiskManager;
        data::OrderRepository &mOrderRepository;

        TradeIDGenerator *mTradeIDGenerator;
        data::DatabaseWorker *mDatabaseWorker;

        static constexpr size_t BATCH_SIZE = 64;
        std::unique_ptr<Command> mCommandBatch[BATCH_SIZE];
    };
}
