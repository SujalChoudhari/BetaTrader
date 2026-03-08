#include "common/Order.h"
#include "data/DatabaseWorker.h"
#include "data/TradeRepository.h"
#include "trading_core/OrderBook.h"
#include "trading_core/RiskManager.h"
#include <gtest/gtest.h>
#include <memory>

// 1. Create a mock for the DatabaseWorker dependency
class MockDatabaseWorker : public data::DatabaseWorker {
public:
    // Call the protected, thread-less constructor of the base class
    MockDatabaseWorker(): data::DatabaseWorker() {}

    // Override enqueue to do nothing, preventing any actual database work
    void enqueue(std::function<void(SQLite::Database&)> task) override {}
};

// 2. Create a mock for the TradeRepository
class MockTradeRepository : public data::TradeRepository {
public:
    // This constructor will now be called with a valid MockDatabaseWorker
    // pointer
    using data::TradeRepository::TradeRepository;

    // We no longer need to override initDatabase because the mock worker's
    // enqueue does nothing

    mutable int addTradeCallCount = 0;
    mutable std::optional<common::Trade> lastAddedTrade;

    // Override the method we want to track
    void addTrade(const common::Trade& trade) override
    {
        addTradeCallCount++;
        lastAddedTrade = trade;
    }
};

// 3. Test Fixture to set up the corrected environment
class RiskManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<MockDatabaseWorker> mockDbWorker;
    std::unique_ptr<MockTradeRepository> mockTradeRepo;
    std::unique_ptr<trading_core::RiskManager> riskManager;
    std::unique_ptr<trading_core::OrderBook> orderBook;
    trading_core::MarketDataPublisher publisher;

    void SetUp() override
    {
        // Create the mock dependencies first
        mockDbWorker = std::make_unique<MockDatabaseWorker>();
        mockTradeRepo
                = std::make_unique<MockTradeRepository>(mockDbWorker.get());

        // Now, create the object under test with valid mock pointers
        riskManager = std::make_unique<trading_core::RiskManager>(
                mockTradeRepo.get());
        orderBook = std::make_unique<trading_core::OrderBook>(
                common::Instrument::EURUSD, publisher);
    }
};

// --- preCheck Tests (unchanged) ---

TEST_F(RiskManagerTest, ValidOrder)
{
    common::Order order(1, 1, common::Instrument::EURUSD, "client1", "client1",
                        common::OrderSide::Buy, common::OrderType::Limit,
                        common::TimeInForce::DAY, 100, 1.1, {});
    ASSERT_TRUE(riskManager->preCheck(order, *orderBook));
}

TEST_F(RiskManagerTest, InvalidQuantity)
{
    common::Order order(1, 1, common::Instrument::EURUSD, "client1", "client1",
                        common::OrderSide::Buy, common::OrderType::Limit,
                        common::TimeInForce::DAY, 0, 1.1, {});
    ASSERT_FALSE(riskManager->preCheck(order, *orderBook));
}

TEST_F(RiskManagerTest, InvalidPrice)
{
    common::Order order(1, 1, common::Instrument::EURUSD, "client1", "client1",
                        common::OrderSide::Buy, common::OrderType::Limit,
                        common::TimeInForce::DAY, 100, 0.0, {});
    ASSERT_FALSE(riskManager->preCheck(order, *orderBook));
}

TEST_F(RiskManagerTest, FatFingerBuyRejection)
{
    common::Order restingSell(1, 1, common::Instrument::EURUSD, "client2",
                              "client2", common::OrderSide::Sell,
                              common::OrderType::Limit,
                              common::TimeInForce::DAY, 100, 1.0, {});
    orderBook->insertOrder(&restingSell);
    common::Order incomingBuy(2, 2, common::Instrument::EURUSD, "client1",
                              "client1", common::OrderSide::Buy,
                              common::OrderType::Limit,
                              common::TimeInForce::DAY, 100, 1.2, {});
    ASSERT_FALSE(riskManager->preCheck(incomingBuy, *orderBook));
}

TEST_F(RiskManagerTest, SelfMatchMarketRejection)
{
    common::Order restingSell(1, 1, common::Instrument::EURUSD, "client1",
                              "client1", common::OrderSide::Sell,
                              common::OrderType::Limit,
                              common::TimeInForce::DAY, 100, 1.1, {});
    orderBook->insertOrder(&restingSell);
    common::Order incomingMarketBuy(
            2, 2, common::Instrument::EURUSD, "client1", "client1",
            common::OrderSide::Buy, common::OrderType::Market,
            common::TimeInForce::DAY, 100, 0.0, {});
    ASSERT_FALSE(riskManager->preCheck(incomingMarketBuy, *orderBook));
}

// --- postTradeUpdate Tests (unchanged) ---

TEST_F(RiskManagerTest, PostTradeUpdateCallsRepository)
{
    common::Trade trade(1, common::Instrument::EURUSD, 10, 11, 100, 1.25, {});
    riskManager->postTradeUpdate(trade);
    ASSERT_EQ(mockTradeRepo->addTradeCallCount, 1);
    ASSERT_TRUE(mockTradeRepo->lastAddedTrade.has_value());
    EXPECT_EQ(mockTradeRepo->lastAddedTrade->getTradeId(), trade.getTradeId());
}

TEST_F(RiskManagerTest, PostTradeUpdateVector)
{
    std::vector<common::Trade> trades;
    common::Trade trade1(1, common::Instrument::EURUSD, 10, 11, 100, 1.25, {});
    common::Trade trade2(2, common::Instrument::GBPUSD, 12, 13, 50, 1.5, {});
    trades.push_back(trade1);
    trades.push_back(trade2);
    riskManager->postTradeUpdate(trades);
    ASSERT_EQ(mockTradeRepo->addTradeCallCount, 2);
}
