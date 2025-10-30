//
// Created by sujal on 30-10-2025.
//

#include <gtest/gtest.h>
#include "trading_core/Partition.h"
#include <chrono>
#include "common/Order.h"
#include "trading_core/Command.h"
#include "trading_core/NewOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/CancelOrder.h"
#include "trading_core/OrderIDGenerator.h"

using namespace trading_core;
using namespace std::chrono_literals;

class PartitionTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset OrderIDGenerator for each test to ensure isolation
        OrderIDGenerator::loadState(); // This will reset currentId to 0 if no state is persisted

        db = std::make_shared<data::DatabaseWorker>(":memory:");
        tradeGen = std::make_shared<TradeIDGenerator>(db);
        execPub = std::make_shared<ExecutionPublisher>();
        partition = std::make_unique<Partition>(common::Instrument::USDCAD, db, tradeGen, execPub);
        partition->start();
        std::this_thread::sleep_for(700ms); // Give worker thread more time to start
    }

    void TearDown() override {
        partition->stop();
        partition.reset();
        execPub.reset();
        tradeGen.reset();
        db.reset();
        std::this_thread::sleep_for(200ms); // Give worker thread more time to fully exit
    }

    static std::shared_ptr<common::Order> createOrder(
        common::OrderSide side = common::OrderSide::Buy,
        common::OrderType type = common::OrderType::Limit,
        double quantity = 100.0,
        double price = 50.0) {
        return std::make_shared<common::Order>(
            OrderIDGenerator::nextId(),
            common::Instrument::EURUSD,
            "CLIENT_" + std::to_string(OrderIDGenerator::getId()),
            side,
            common::OrderType::Limit,
            quantity,
            price,
            std::chrono::system_clock::now()
        );
    }

    data::DatabaseWorkerPtr db;
    std::shared_ptr<TradeIDGenerator> tradeGen;
    std::shared_ptr<ExecutionPublisher> execPub;
    std::unique_ptr<Partition> partition;
};

TEST_F(PartitionTestFixture, EnqueueNewOrder) {
    auto order = createOrder();
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
    EXPECT_NO_THROW(partition->enqueue(std::move(newOrder)));
}

TEST_F(PartitionTestFixture, EnqueueModifyOrder) {
    auto order = createOrder();
    // Enqueue a new order first so it exists to be modified
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
    partition->enqueue(std::move(newOrder));
    std::this_thread::sleep_for(50ms); // Give worker time to process new order

    auto modifyOrder = std::make_unique<ModifyOrder>(std::chrono::system_clock::now(), order->getClientId(),
                                                     order->getId(), 80.0, 52.0);
    EXPECT_NO_THROW(partition->enqueue(std::move(modifyOrder)));
}

TEST_F(PartitionTestFixture, EnqueueCancelOrder) {
    auto order = createOrder();
    // Enqueue a new order first so it exists to be cancelled
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
    partition->enqueue(std::move(newOrder));
    std::this_thread::sleep_for(50ms); // Give worker time to process new order

    auto cancelOrder = std::make_unique<CancelOrder>(order->getClientId(), std::chrono::system_clock::now(),
                                                     order->getId());
    EXPECT_NO_THROW(partition->enqueue(std::move(cancelOrder)));
}

TEST_F(PartitionTestFixture, EnqueueAndDrop) {
    // Fill the queue
    for (int i = 0; i < 4096; ++i) {
        auto order = createOrder();
        auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
        partition->enqueue(std::move(newOrder));
    }

    // This should drop the command (queue full)
    auto order = createOrder();
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
    EXPECT_NO_THROW(partition->enqueue(std::move(newOrder)));
}
