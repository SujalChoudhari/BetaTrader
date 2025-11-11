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
        data::OrderRepository &orderRepository,
        TradeIDGenerator *tradeIDGenerator,
        data::DatabaseWorker *databaseWorker
    )
        : mCommandQueue(commandQueue)
          , mOrderManager(orderManager)
          , mOrderBook(orderBook)
          , mMatcher(matcher)
          , mRiskManager(riskManager)
          , mOrderRepository(orderRepository)
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
            processNextCommand();
        }

        LOG_INFO("Worker Thread exited");
    }

    void WorkerThread::processNextCommand() {
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


    void WorkerThread::processNewOrder(NewOrder &cmd) const {
        const auto order = cmd.getOrder();

        if (!mRiskManager.preCheck(*order, mOrderBook)) {
            ExecutionPublisher::publishRejection(
                order->getId(),
                order->getClientId(),
                "Risk check failed"
            );
            return;
        }

        mOrderManager.addOrder(cmd.releaseOrder());
        mOrderRepository.saveOrder(*order);
        mOrderBook.insertOrder(order);

        for (const auto trades = mMatcher.match(order, mOrderBook); auto &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            mOrderRepository.updateOrder(*mOrderManager.getOrderById(trade.getBuyOrderId()).value());
            mOrderRepository.updateOrder(*mOrderManager.getOrderById(trade.getSellOrderId()).value());
            ExecutionPublisher::publishTrade(trade);
        }

        if (order->getRemainingQuantity() == 0 || order->getOrderType() == common::OrderType::Market) {
            mOrderRepository.removeOrder(order->getId());
        }

        ExecutionPublisher::publishExecution(*order, "NEW");
    }

    void WorkerThread::processCancelOrder(const CancelOrder &cmd) const {
        const auto orderOpt = mOrderManager.getOrderById(cmd.getOrderId());
        if (!orderOpt) {
            ExecutionPublisher::publishRejection(cmd.getOrderId(), cmd.getClientId(),
                                                 "Order not found");
            return;
        }
        const auto order = *orderOpt;

        if (mOrderBook.cancelOrder(cmd.getOrderId())) {
            ExecutionPublisher::publishExecution(*order, "CANCELED");
            mOrderRepository.removeOrder(cmd.getOrderId());
            mOrderManager.removeOrderById(cmd.getOrderId());
        } else {
            ExecutionPublisher::publishRejection(cmd.getOrderId(), cmd.getClientId(),
                                                 "Order cannot be canceled");
        }
    }

    void WorkerThread::processModifyOrder(const ModifyOrder &cmd) const {
        const auto orderOpt = mOrderManager.getOrderById(cmd.getOrderId());
        if (!orderOpt) {
            ExecutionPublisher::publishRejection(cmd.getOrderId(), cmd.getClientId(),
                                                 "Order not found");
            return;
        }
        const auto order = *orderOpt;

        mOrderBook.cancelOrder(cmd.getOrderId());
        mOrderRepository.removeOrder(cmd.getOrderId());

        order->setPrice(cmd.getNewPrice());
        order->setOriginalQuantity(cmd.getNewQuantity());

        mOrderRepository.saveOrder(*order);
        mOrderBook.insertOrder(order);

        for (const auto trades = mMatcher.match(order, mOrderBook); auto &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            mOrderRepository.updateOrder(*mOrderManager.getOrderById(trade.getBuyOrderId()).value());
            mOrderRepository.updateOrder(*mOrderManager.getOrderById(trade.getSellOrderId()).value());
            ExecutionPublisher::publishTrade(trade);
        }

        if (order->getRemainingQuantity() == 0) {
            mOrderRepository.removeOrder(order->getId());
        }

        ExecutionPublisher::publishExecution(*order, "REPLACED");
    }
}
