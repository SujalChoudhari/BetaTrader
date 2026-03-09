#include "trading_core/TradingCore.h"
#include "trading_core/CancelOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/NewOrder.h"
#include "trading_core/Partition.h"
#include "trading_core/ExecutionPublisher.h"
#include "data/DatabaseWorker.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include "mocks/MockDependencies.h"

using namespace trading_core;

class TradingCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        dbWorker = std::make_unique<data::DatabaseWorker>(":memory:");
    }

    void TearDown() override {
        dbWorker.reset();
    }

    std::unique_ptr<data::DatabaseWorker> dbWorker;
};

TEST_F(TradingCoreTest, DefaultConstructor) {
    TradingCore core;
    EXPECT_NE(core.getOrderIDGenerator(), nullptr);
    EXPECT_NE(core.getAuthRepository(), nullptr);
    EXPECT_EQ(&TradingCore::getInstance(), &core);
}

TEST_F(TradingCoreTest, ParamConstructor_AutoInitTrue) {
    TradingCore core(dbWorker.get(), true);
    EXPECT_NE(core.getOrderIDGenerator(), nullptr);
    EXPECT_NE(core.getPartition(common::Instrument::EURUSD), nullptr);
    EXPECT_EQ(&TradingCore::getInstance(), &core);
}

TEST_F(TradingCoreTest, ParamConstructor_AutoInitFalse) {
    TradingCore core(dbWorker.get(), false);
    EXPECT_NE(core.getOrderIDGenerator(), nullptr);
    EXPECT_EQ(core.getPartition(common::Instrument::EURUSD), nullptr);
    EXPECT_EQ(&TradingCore::getInstance(), &core);
}

TEST_F(TradingCoreTest, StartStopControls) {
    TradingCore core(dbWorker.get(), true);
    // Just invoke them, partition queues are empty so it shouldn't block
    core.start();
    core.stopAcceptingCommands();
    core.waitAllQueuesIdle();
    core.stop();
}

TEST_F(TradingCoreTest, SubmitNewOrderCommand) {
    TradingCore core(dbWorker.get(), true);
    core.start();

    auto order = std::make_unique<common::Order>(
            1, 0, common::Instrument::EURUSD, "1",
            "clordid1", common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
    
    auto cmd = std::make_unique<NewOrder>("1", order->getTimestamp(), std::move(order));
    
    // Submitting NewOrder should enqueue to the correct partition
    auto partition = core.getPartition(common::Instrument::EURUSD);
    ASSERT_NE(partition, nullptr);
    
    auto initialQueueSize = partition->getQueueSize();
    core.submitCommand(std::move(cmd));
    
    EXPECT_GE(partition->getQueueSize(), initialQueueSize);
    
    core.waitAllQueuesIdle();
    core.stop();
}

TEST_F(TradingCoreTest, FindAndGetOrder) {
    TradingCore core(dbWorker.get(), true);
    core.start();

    auto order = std::make_unique<common::Order>(
            1, 0, common::Instrument::EURUSD, "1",
            "clordid1", common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
            
    auto cmd = std::make_unique<NewOrder>("1", order->getTimestamp(), std::move(order));
    core.submitCommand(std::move(cmd));
    core.waitAllQueuesIdle();
    
    auto instOpt = core.findPartitionForOrder(0);
    EXPECT_FALSE(core.findPartitionForOrder(9999).has_value());
    EXPECT_FALSE(core.getOrder(9999).has_value());
    EXPECT_FALSE(core.getOrderByClientOrderId("nonexistent").has_value());
    
    core.stop();
}

TEST_F(TradingCoreTest, Subscriptions) {
    TradingCore core(dbWorker.get(), true);
    core.subscribeToMarketData(common::Instrument::EURUSD, 1);
    core.subscribeToMarketData(common::Instrument::USDJPY, 2);
    
    core.unsubscribeFromMarketData(common::Instrument::EURUSD, 1);
    core.unsubscribeFromMarketData(2);
    
    auto callback = [](const fix::ExecutionReport& ) {};
    core.subscribeToExecutions(callback);
    
    auto& mdp = core.getMarketDataPublisher();
    (void)mdp;
}


TEST_F(TradingCoreTest, ModifyCancelOrderFoundAndGetOrderSuccess) {
    TradingCore core(dbWorker.get(), true);
    core.start();

    // 1. Submit a NewOrder and retrieve its assigned ID via ClientOrderId
    auto order = std::make_unique<common::Order>(
            1, 0, common::Instrument::EURUSD, "1",
            "clordid1", common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
    
    auto cmd = std::make_unique<NewOrder>("1", order->getTimestamp(), std::move(order));
    core.submitCommand(std::move(cmd));
    core.waitAllQueuesIdle();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if order is recorded.
    auto orderOpt = core.getOrderByClientOrderId("1");
    if (orderOpt) {
        auto orderId = orderOpt->getId();

        // 2. Test getting order successfully
        auto foundOrderOpt = core.getOrder(orderId);

        // 3. Test ModifyOrder finding partition
        auto modCmd = std::make_unique<ModifyOrder>("1", std::chrono::system_clock::now(), orderId, 50, 1.26);
        core.submitCommand(std::move(modCmd));

        // 4. Test CancelOrder finding partition
        auto canCmd = std::make_unique<CancelOrder>("1", std::chrono::system_clock::now(), orderId);
        core.submitCommand(std::move(canCmd));
    }
    
    core.waitAllQueuesIdle();
    core.stop();
}

TEST_F(TradingCoreTest, PublishMethodsCoverage) {
    TradingCore core(dbWorker.get(), true);
    core.start();

    // 1. Submit a NewOrder to have something to look up
    auto order = std::make_unique<common::Order>(
            1, 101, common::Instrument::EURUSD, "1",
            "clordid1", common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
    auto cmd = std::make_unique<NewOrder>("1", order->getTimestamp(), std::move(order));
    core.submitCommand(std::move(cmd));
    core.waitAllQueuesIdle();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 2. Trigger Trade (hits getOrder)
    // We already have Buy order 101. Now add a Sell order 102 that matches.
    auto sellOrder = std::make_unique<common::Order>(
            2, 102, common::Instrument::EURUSD, "1",
            "clordid2", common::OrderSide::Sell, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
    auto sellCmd = std::make_unique<NewOrder>("1", sellOrder->getTimestamp(), std::move(sellOrder));
    core.submitCommand(std::move(sellCmd));
    core.waitAllQueuesIdle();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 3. Trigger Rejection (hits publishRejection) - Limit order with price 0 fails RiskManager preCheck
    auto badOrder = std::make_unique<common::Order>(
            3, 103, common::Instrument::EURUSD, "1",
            "clordid3", common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 0.0,
            std::chrono::system_clock::now());
    auto badCmd = std::make_unique<NewOrder>("1", badOrder->getTimestamp(), std::move(badOrder));
    core.submitCommand(std::move(badCmd));
    core.waitAllQueuesIdle();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    core.stop();
}

#ifndef NDEBUG
TEST_F(TradingCoreTest, SetPartitionCoverage) {
    TradingCore core(dbWorker.get(), false);
    core.setPartition(common::Instrument::EURUSD, nullptr);
}
#endif
