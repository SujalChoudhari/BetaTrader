#include "trading_core/WorkerThread.h"
#include <iostream>

#include "logging/Logger.h"
#include "trading_core/TradingCoreRunbookDefinations.h"

namespace trading_core {
    WorkerThread::WorkerThread(
        rigtorp::SPSCQueue<std::unique_ptr<Command>> &commandQueue,
        OrderManager &orderManager,
        OrderBook &orderBook,
        Matcher &matcher,
        RiskManager &riskManager,
        std::shared_ptr<ExecutionPublisher> executionPublisher,
        std::shared_ptr<TradeIDGenerator> tradeIDGenerator,
        data::DatabaseWorkerPtr databaseWorker
    )
        : mRunning(false)
          , mCommandQueue(commandQueue)
          , mOrderManager(orderManager)
          , mOrderBook(orderBook)
          , mMatcher(matcher)
          , mRiskManager(riskManager)
          , mExecutionPublisher(std::move(executionPublisher))
          , mTradeIDGenerator(std::move(tradeIDGenerator))
          , mDatabaseWorker(std::move(databaseWorker))
          , mCommandBatch{} {
    }

    WorkerThread::~WorkerThread() {
        if (mRunning.load(std::memory_order_acquire)) {
            stop();
        }
    }

    void WorkerThread::start() {
        LOG_INFO("Worker Thread attempting to start");
        mRunning.store(true, std::memory_order_release);
        mThread = std::thread(&WorkerThread::runLoop, this);
    }

    void WorkerThread::stop() {
        LOG_INFO("Worker thread attempting to exit");
        mRunning.store(false, std::memory_order_release);
        if (mThread.joinable()) {
            mThread.join();
        }
    }

    void WorkerThread::runLoop() {
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(5));
        LOG_INFO("Worker Thread started");
        while (mRunning.load(std::memory_order_acquire)) {
            size_t dequeued = 0;
            for (size_t i = 0; i < BATCH_SIZE; ++i) {
                if (mCommandQueue.front()) {
                    mCommandBatch[i] = std::move(*mCommandQueue.front());
                    mCommandQueue.pop();
                    dequeued++;
                } else {
                    break;
                }
            }

            if (dequeued > 0) {
                processBatch(mCommandBatch, dequeued);
            } else {
                std::this_thread::yield();
            }
        }

        LOG_INFO("Worker Thread exited");
    }

    void WorkerThread::processBatch(std::unique_ptr<Command> *commands, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            auto& cmd = commands[i];
            switch (cmd->getType()) {
                case CommandType::NewOrder: processNewOrder(*static_cast<NewOrder *>(cmd.get()));
                    break;
                case CommandType::CancelOrder: processCancelOrder(*static_cast<CancelOrder *>(cmd.get()));
                    break;
                case CommandType::ModifyOrder: processModifyOrder(*static_cast<ModifyOrder *>(cmd.get()));
                    break;
                default: LOG_ERROR(errors::ETRADE1, "Found type: {}", trading_core::to_string(cmd->getType()));
                    break;
            }
        }
    }


    void WorkerThread::processNewOrder(const NewOrder &cmd) {
        auto order = cmd.getOrder();

        if (!RiskManager::preCheck(*order)) {
            ExecutionPublisher::publishRejection(
                order->getId(),
                order->getClientId(),
                "Risk check failed"
            );
            return;
        }

        mOrderManager.addOrder(order);
        mOrderBook.insertOrder(order);

        auto trades = mMatcher.match(order, mOrderBook);
        for (auto &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            ExecutionPublisher::publishTrade(trade);
        }

        ExecutionPublisher::publishExecution(*order, "NEW");
    }

    void WorkerThread::processCancelOrder(const CancelOrder &cmd) {
        auto orderOpt = mOrderManager.getOrderById(cmd.getOrderId());
        if (!orderOpt) {
            ExecutionPublisher::publishRejection(cmd.getOrderId(), cmd.getClientId(),
                                                 "Order not found");
            return;
        }
        auto order = *orderOpt;

        if (mOrderBook.cancelOrder(cmd.getOrderId())) {
            mOrderManager.removeOrderById(cmd.getOrderId());
            ExecutionPublisher::publishExecution(*order, "CANCELED");
        } else {
            ExecutionPublisher::publishRejection(cmd.getOrderId(), cmd.getClientId(),
                                                 "Order cannot be canceled");
        }
    }

    void WorkerThread::processModifyOrder(const ModifyOrder &cmd) {
        auto orderOpt = mOrderManager.getOrderById(cmd.getOrderId());
        if (!orderOpt) {
            ExecutionPublisher::publishRejection(cmd.getOrderId(), cmd.getClientId(),
                                                 "Order not found");
            return;
        }
        auto order = *orderOpt;

        mOrderBook.cancelOrder(cmd.getOrderId());

        order->setPrice(cmd.getNewPrice());
        order->setOriginalQuantity(cmd.getNewQuantity());

        mOrderBook.insertOrder(order);

        auto trades = mMatcher.match(order, mOrderBook);
        for (auto &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            ExecutionPublisher::publishTrade(trade);
        }

        ExecutionPublisher::publishExecution(*order, "REPLACED");
    }
}
