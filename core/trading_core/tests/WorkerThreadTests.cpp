//
// Created by sujal on 30-10-2025.
//

#include <gtest/gtest.h>
#include "trading_core/WorkerThread.h"
#include "trading_core/CommandType.h"
#include "trading_core/OrderManager.h"
#include "trading_core/OrderBook.h"
#include "trading_core/Matcher.h"
#include "trading_core/RiskManager.h"
#include "trading_core/ExecutionPublisher.h"
#include "trading_core/TradeIDGenerator.h"
#include "data/DatabaseWorker.h"
#include "trading_core/NewOrder.h"

using namespace trading_core;
using namespace common;
using namespace std::chrono_literals;

class DummyOrder : public common::Order {
public:
    DummyOrder()
        : Order(1, Instrument::USDJPY, "C1", common::OrderSide::Buy,
                common::OrderType::Limit, 100, 10.0, std::chrono::system_clock::now()) {
    }
};

TEST(WorkerThreadTest, StartAndStopThread) {
    rigtorp::SPSCQueue<std::unique_ptr<Command>> queue(1024);
    auto dbWorker = std::make_shared<data::DatabaseWorker>(":memory:");
    auto tradeGen = std::make_shared<TradeIDGenerator>(dbWorker);
    auto execPub = std::make_shared<ExecutionPublisher>();

    OrderManager orderManager;
    OrderBook orderBook;
    Matcher matcher(dbWorker);
    RiskManager riskManager(dbWorker);

    WorkerThread worker(queue, orderManager, orderBook, matcher,
                        riskManager, execPub, tradeGen, dbWorker);

    EXPECT_NO_THROW(worker.start());
    std::this_thread::sleep_for(10ms);
    EXPECT_NO_THROW(worker.stop());
}

TEST(WorkerThreadTest, EnqueueAndProcessBasicCommand) {
    rigtorp::SPSCQueue<std::unique_ptr<Command>> queue(1024);
    auto dbWorker = std::make_shared<data::DatabaseWorker>(":memory:");
    auto tradeGen = std::make_shared<TradeIDGenerator>(dbWorker);
    auto execPub = std::make_shared<ExecutionPublisher>();

    OrderManager orderManager;
    OrderBook orderBook;
    Matcher matcher(dbWorker);
    RiskManager riskManager(dbWorker);

    WorkerThread worker(queue, orderManager, orderBook, matcher,
                        riskManager, execPub, tradeGen, dbWorker);

    worker.start();

    auto order = std::make_shared<DummyOrder>();
    auto newOrderCmd = std::make_unique<NewOrder>("C1", std::chrono::system_clock::now(), order);

    queue.push(std::move(newOrderCmd));

    std::this_thread::sleep_for(50ms);

    worker.stop();

    ASSERT_TRUE(orderManager.containsOrderById(1));
}
