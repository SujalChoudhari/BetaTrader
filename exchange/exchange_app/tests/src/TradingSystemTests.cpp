#include "logging/Logger.h" // Include the logger
#include "MockDependencies.h"
#include "TestUtils.h"
#include "spdlog/sinks/ostream_sink.h" // Include for ostream_sink
#include <exchange_routing/CancelOrder.h>
#include <exchange_routing/ModifyOrder.h>
#include <exchange_routing/NewOrder.h>
#include <exchange_routing/Partition.h>
#include <exchange_app/TradingCore.h>
#include <exchange_routing/WorkerThread.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <thread>
//

class TradingSystemTest : public ::testing::Test {
protected:
    // Dependencies
    std::unique_ptr<MockDatabaseWorker> mockDbWorker;
    std::unique_ptr<MockTradeIDRepository> mockTradeIdRepo;
    std::unique_ptr<MockTradeIDGenerator> mockTradeIdGenerator;
    std::unique_ptr<MockMarketDataPublisher> mockMdPublisher;

    // Object Under Test
    std::unique_ptr<trading_core::TradingCore> tradingCore;

    // For capturing log output
    std::stringstream log_buffer;

    void SetUp() override
    {
        // Set up a custom sink for the logger
        auto ostream_sink
                = std::make_shared<spdlog::sinks::ostream_sink_mt>(log_buffer);
        logging::Logger::Init("test_logger", "logs/test.log", false, false,
                              spdlog::level::trace, 8192, 1, 1024 * 1024 * 5, 2,
                              ostream_sink);

        mockDbWorker = std::make_unique<MockDatabaseWorker>();
        mockTradeIdRepo = std::make_unique<MockTradeIDRepository>(mockDbWorker.get());
        mockTradeIdGenerator
                = std::make_unique<MockTradeIDGenerator>(mockTradeIdRepo.get());
        mockMdPublisher = std::make_unique<MockMarketDataPublisher>();

        // Create a TradingCore without auto-initializing partitions
        tradingCore = std::make_unique<trading_core::TradingCore>(
                mockDbWorker.get(), false);
    }

    void TearDown() override
    {
        if (tradingCore) {
            tradingCore->stop();
            tradingCore.reset();
        }
        logging::Logger::Shutdown();
    }

#ifndef NDEBUG
    void setupMockPartition(common::Instrument instrument)
    {
        auto mockTradeRepo
                = std::make_unique<MockTradeRepository>(mockDbWorker.get());
        auto mockOrderRepo
                = std::make_unique<MockOrderRepository>(mockDbWorker.get());
        auto mockOrderManager = std::make_unique<MockOrderManager>();
        auto mockOrderBook = std::make_unique<MockOrderBook>(instrument, *mockMdPublisher);
        auto mockMatcher = std::make_unique<MockMatcher>();
        auto mockRiskManager = std::make_unique<MockRiskManager>();

        auto mockPartition = std::make_unique<trading_core::Partition>(
                instrument, mockDbWorker.get(), mockTradeIdGenerator.get(),
                *mockMdPublisher,
                std::move(mockTradeRepo), std::move(mockOrderRepo),
                std::move(mockOrderManager), std::move(mockOrderBook),
                std::move(mockMatcher), std::move(mockRiskManager));
        tradingCore->setPartition(instrument, std::move(mockPartition));
    }
#endif
};

TEST_F(TradingSystemTest, SystemInitialization)
{
    ASSERT_TRUE(tradingCore != nullptr);
}

#ifndef NDEBUG
TEST_F(TradingSystemTest, SubmitNewOrderEndToEnd)
{
    setupMockPartition(common::Instrument::EURUSD);
    tradingCore->start();

    auto order = std::make_unique<common::Order>(
            123, 456, common::Instrument::EURUSD, "test_client", "SENDER",
            common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
    auto cmd = std::make_unique<trading_core::NewOrder>(
            "test_client", order->getTimestamp(), std::move(order));

    tradingCore->submitCommand(std::move(cmd));
    tradingCore->waitAllQueuesIdle();
    // Add a small delay to allow the logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string output = log_buffer.str();
    // Verify that the execution report was published to the logger
    ASSERT_NE(output.find("EXECUTION | Action=NEW"), std::string::npos);
    ASSERT_NE(output.find("OrderID=456"), std::string::npos);
    ASSERT_NE(output.find("Client=test_client"), std::string::npos);
}

TEST_F(TradingSystemTest, SubmitNewOrderRoutesToCorrectPartition)
{
    setupMockPartition(common::Instrument::USDJPY);
    setupMockPartition(common::Instrument::EURUSD);

    // Don't start the core, so the command stays in the queue
    auto order = std::make_unique<common::Order>(
            123, 789, common::Instrument::USDJPY, "test_client", "SENDER",
            common::OrderSide::Buy, common::OrderType::Limit,
            common::TimeInForce::DAY, 100, 1.25,
            std::chrono::system_clock::now());
    auto cmd = std::make_unique<trading_core::NewOrder>(
            "test_client", order->getTimestamp(), std::move(order));

    tradingCore->submitCommand(std::move(cmd));

    // Check the queue sizes
    EXPECT_EQ(tradingCore->getPartition(common::Instrument::USDJPY)
                      ->getQueueSize(),
              1);
    EXPECT_EQ(tradingCore->getPartition(common::Instrument::EURUSD)
                      ->getQueueSize(),
              0);
}
#endif
