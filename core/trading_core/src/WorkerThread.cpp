#include "trading_core/WorkerThread.h"
#include <iostream>

#include "logging/Logger.h"
#include "trading_core/TradingCoreRunbookDefinations.h"

using namespace std::chrono_literals;

namespace trading_core {
    WorkerThread::WorkerThread(
        rigtorp::SPSCQueue<std::unique_ptr<Command> > &commandQueue,
        OrderManager &orderManager,
        OrderBook &orderBook,
        Matcher &matcher,
        RiskManager &riskManager,
        TradeIDGenerator *tradeIDGenerator,
        data::DatabaseWorker *databaseWorker
    )
        : mCommandQueue(commandQueue)
          , mOrderManager(orderManager)
          , mOrderBook(orderBook)
          , mMatcher(matcher)
          , mRiskManager(riskManager)
          , mTradeIDGenerator(tradeIDGenerator)
          , mDatabaseWorker(databaseWorker)
          , mCommandBatch{} {
    }

    WorkerThread::~WorkerThread() {
        if (mThread.joinable()) {
            stop();
        }
    }

    void WorkerThread::start() {
        LOG_INFO("Worker Thread attempting to start");
        mThread = std::jthread([this](std::stop_token st) { this->runLoop(st); });
    }

    void WorkerThread::stop() {
        LOG_INFO("Worker thread attempting to exit");
        mThread.request_stop();
        if (mThread.joinable()) {
            mThread.join();
        }
    }

    void WorkerThread::runLoop(std::stop_token stopToken) {
        std::this_thread::sleep_for(5ms);
        LOG_INFO("Worker Thread started");
        while (!stopToken.stop_requested()) {
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
            auto &cmd = commands[i];
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


    void WorkerThread::processNewOrder(NewOrder &cmd) {
        auto order = cmd.getOrder();

        if (!mRiskManager.preCheck(*order, mOrderBook)) {
            ExecutionPublisher::publishRejection(
                order->getId(),
                order->getClientId(),
                "Risk check failed"
            );
            return;
        }

        mOrderBook.insertOrder(order);

        auto trades = mMatcher.match(order, mOrderBook);
        for (auto &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            ExecutionPublisher::publishTrade(trade);
        }

        ExecutionPublisher::publishExecution(*order, "NEW");
        mOrderManager.addOrder(cmd.releaseOrder());
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
