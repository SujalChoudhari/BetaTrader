#include <gtest/gtest.h>
#include "trading_core/TradingCore.h"
#include "trading_core/WorkerThread.h"
#include "trading_core/Partition.h"
#include "trading_core/NewOrder.h"
#include "trading_core/CancelOrder.h"
#include "trading_core/ModifyOrder.h"
#include "mocks/MockDependencies.h"
#include <sstream>
#include <iostream>
#include <thread>

// Helper class to temporarily redirect cout
class CoutRedirector {
public:
    CoutRedirector(std::stringstream& new_buffer)
        : old_buffer(std::cout.rdbuf(new_buffer.rdbuf())) {}

    ~CoutRedirector() {
        std::cout.rdbuf(old_buffer);
    }

private:
    std::streambuf* old_buffer;
};

class TradingSystemTest : public ::testing::Test {
protected:
    // Dependencies
    std::unique_ptr<MockDatabaseWorker> mockDbWorker;
    std::unique_ptr<MockTradeIDGenerator> mockTradeIdGenerator;

    // Object Under Test
    std::unique_ptr<trading_core::TradingCore> tradingCore;

    void SetUp() override {
        mockDbWorker = std::make_unique<MockDatabaseWorker>();
        mockTradeIdGenerator = std::make_unique<MockTradeIDGenerator>(mockDbWorker.get());

        // Create a TradingCore without auto-initializing partitions
        tradingCore = std::make_unique<trading_core::TradingCore>(mockDbWorker.get(), false);
    }

    void TearDown() override {
        if (tradingCore) {
            tradingCore->stop();
        }
    }

    void setupMockPartition(common::Instrument instrument) {
        auto mockTradeRepo = std::make_unique<MockTradeRepository>(mockDbWorker.get());
        auto mockOrderRepo = std::make_unique<MockOrderRepository>(mockDbWorker.get());
        auto mockOrderManager = std::make_unique<MockOrderManager>();
        auto mockOrderBook = std::make_unique<MockOrderBook>();
        auto mockMatcher = std::make_unique<MockMatcher>();
        auto mockRiskManager = std::make_unique<MockRiskManager>();

        auto mockPartition = std::make_unique<trading_core::Partition>(
            instrument,
            mockDbWorker.get(),
            mockTradeIdGenerator.get(),
            std::move(mockTradeRepo),
            std::move(mockOrderRepo),
            std::move(mockOrderManager),
            std::move(mockOrderBook),
            std::move(mockMatcher),
            std::move(mockRiskManager)
        );
        tradingCore->setPartition(instrument, std::move(mockPartition));
    }
};

TEST_F(TradingSystemTest, SystemInitialization) {
    ASSERT_TRUE(tradingCore != nullptr);
}

TEST_F(TradingSystemTest, SubmitNewOrderEndToEnd) {
    setupMockPartition(common::Instrument::EURUSD);
    tradingCore->start();

    auto order = std::make_unique<common::Order>(123, common::Instrument::EURUSD, "test_client", common::OrderSide::Buy, common::OrderType::Limit, 100, 1.25, std::chrono::system_clock::now());
    auto cmd = std::make_unique<trading_core::NewOrder>("test_client", order->getTimestamp(), std::move(order));

    std::stringstream buffer;
    {
        CoutRedirector redirect(buffer);
        tradingCore->submitCommand(std::move(cmd));
        tradingCore->waitUntilIdle();
        // Add a small delay to allow the publisher to print
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::string output = buffer.str();
    // Verify that the execution report was published to cout
    ASSERT_NE(output.find("[ExecutionPublisher] EXECUTION | Action=NEW"), std::string::npos);
    ASSERT_NE(output.find("OrderID=123"), std::string::npos);
    ASSERT_NE(output.find("Client=test_client"), std::string::npos);
}

TEST_F(TradingSystemTest, SubmitNewOrderRoutesToCorrectPartition) {
    setupMockPartition(common::Instrument::USDJPY);
    setupMockPartition(common::Instrument::EURUSD);

    // Don't start the core, so the command stays in the queue
    auto order = std::make_unique<common::Order>(123, common::Instrument::USDJPY, "test_client", common::OrderSide::Buy, common::OrderType::Limit, 100, 1.25, std::chrono::system_clock::now());
    auto cmd = std::make_unique<trading_core::NewOrder>("test_client", order->getTimestamp(), std::move(order));

    tradingCore->submitCommand(std::move(cmd));

    // Check the queue sizes
    EXPECT_EQ(tradingCore->getPartition(common::Instrument::USDJPY)->getQueueSize(), 1);
    EXPECT_EQ(tradingCore->getPartition(common::Instrument::EURUSD)->getQueueSize(), 0);
}
