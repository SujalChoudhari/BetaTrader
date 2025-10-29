#include "trading_core/WorkerThread.h"
#include <iostream>

namespace trading_core {
    WorkerThread::WorkerThread(
        rigtorp::SPSCQueue<Command *> &commandQueue,
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
        mRunning.store(true, std::memory_order_release);
        mThread = std::thread(&WorkerThread::runLoop, this);
    }

    void WorkerThread::stop() {
        mRunning.store(false, std::memory_order_release);
        if (mThread.joinable()) {
            mThread.join();
        }
    }

    void WorkerThread::runLoop() {
        std::cout << "[WorkerThread] Started event loop\n";

        while (mRunning.load(std::memory_order_acquire)) {
            size_t dequeued = 0;

            for (size_t i = 0; i < BATCH_SIZE; ++i) {
                Command **cmd = mCommandQueue.front();
                if (!cmd) break;
                mCommandBatch[i] = *cmd;
                mCommandQueue.pop();
                ++dequeued;
            }

            if (dequeued > 0) {
                processBatch(mCommandBatch, dequeued);
            } else {
                std::this_thread::yield();
            }
        }

        std::cout << "[WorkerThread] Stopped event loop\n";
    }

    void WorkerThread::processBatch(Command **commands, size_t count) const {
        for (size_t i = 0; i < count; ++i) {
            Command *cmd = commands[i];
            switch (cmd->getType()) {
                case CommandType::NewOrder:
                    processNewOrder(static_cast<NewOrder *>(cmd));
                    break;
                case CommandType::CancelOrder:
                    processCancelOrder(static_cast<CancelOrder *>(cmd));
                    break;
                case CommandType::ModifyOrder:
                    processModifyOrder(static_cast<ModifyOrder *>(cmd));
                    break;
                default:
                    std::cerr << "[WorkerThread] Unknown command type: "
                            << static_cast<int>(cmd->getType()) << "\n";
                    break;
            }
            delete cmd;
        }
    }

    void WorkerThread::processNewOrder(const NewOrder *cmd) const {
        common::Order *order = const_cast<common::Order *>(&cmd->getOrder());

        if (!mRiskManager.preCheck(*order)) {
            mExecutionPublisher->publishRejection(
                order->getId(),
                order->getClientId(),
                "Risk check failed"
            );
            return;
        }

        mOrderManager.addOrder(order);
        mOrderBook.insertOrder(order);

        for (std::vector<common::Trade> trades = mMatcher.match(*order, mOrderBook);
             common::Trade &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            mExecutionPublisher->publishTrade(trade);
        }

        mExecutionPublisher->publishExecution(*order, "NEW");
    }

    void WorkerThread::processCancelOrder(const CancelOrder *cmd) const {
        const common::Order *order = mOrderManager.getOrderById(cmd->getOrderId()).value_or(nullptr);
        if (!order) {
            mExecutionPublisher->publishRejection(cmd->getOrderId(), cmd->getClientId(), "Order not found");
            return;
        }

        if (bool removed = mOrderBook.cancelOrder(cmd->getOrderId())) {
            mOrderManager.removeOrderById(cmd->getOrderId());
            mExecutionPublisher->publishExecution(*order, "CANCELED");
        } else {
            mExecutionPublisher->publishRejection(cmd->getOrderId(), cmd->getClientId(), "Order cannot be canceled");
        }
    }

    void WorkerThread::processModifyOrder(const ModifyOrder *cmd) const {
        common::Order *order = mOrderManager.getOrderById(cmd->getOrderId()).value_or(nullptr);
        if (!order) {
            mExecutionPublisher->publishRejection(cmd->getOrderId(), cmd->getClientId(), "Order not found");
            return;
        }

        mOrderBook.cancelOrder(cmd->getOrderId());

        order->setPrice(cmd->getNewPrice());
        order->setOriginalQuantity(cmd->getNewQuantity());

        mOrderBook.insertOrder(order);

        for (std::vector<common::Trade> trades = mMatcher.match(*order, mOrderBook);
             common::Trade &trade: trades) {
            mRiskManager.postTradeUpdate(trade);
            mExecutionPublisher->publishTrade(trade);
        }

        mExecutionPublisher->publishExecution(*order, "REPLACED");
    }
}
