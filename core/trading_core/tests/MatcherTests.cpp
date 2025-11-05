#include <gtest/gtest.h>
#include "trading_core/Matcher.h"
#include "trading_core/OrderBook.h"
#include "trading_core/TradeIDGenerator.h"
#include "data/DatabaseWorker.h"
#include "common/Order.h"
#include <memory>
#include <vector>

// 1. Mock the lowest-level dependency: DatabaseWorker
class MockDatabaseWorker : public data::DatabaseWorker {
public:
    MockDatabaseWorker() : data::DatabaseWorker() {}
    void enqueue(std::function<void(SQLite::Database&)> task) override {}
};

// 2. Mock TradeIDGenerator
class MockTradeIDGenerator : public trading_core::TradeIDGenerator {
public:
    // The constructor now takes the mock worker and passes it to the base class
    explicit MockTradeIDGenerator(data::DatabaseWorker* dbWorker) : trading_core::TradeIDGenerator(dbWorker) {}

    // Override base class methods that interact with the repository to do nothing
    void saveState() override {}
    void loadState() override {}

    common::TradeID nextId() override {
        return ++mock_current_id;
    }
private:
    common::TradeID mock_current_id = 0;
};

// 3. Test Fixture to manage the corrected environment
class MatcherTest : public ::testing::Test {
protected:
    std::unique_ptr<MockDatabaseWorker> mockDbWorker;
    std::unique_ptr<MockTradeIDGenerator> mockTradeIdGenerator;
    std::unique_ptr<trading_core::Matcher> matcher;
    std::unique_ptr<trading_core::OrderBook> orderBook;
    std::vector<std::unique_ptr<common::Order>> orderStore;

    void SetUp() override {
        mockDbWorker = std::make_unique<MockDatabaseWorker>();
        mockTradeIdGenerator = std::make_unique<MockTradeIDGenerator>(mockDbWorker.get());
        matcher = std::make_unique<trading_core::Matcher>(mockTradeIdGenerator.get());
        orderBook = std::make_unique<trading_core::OrderBook>();
    }

    common::Order* createOrder(common::OrderID id, common::OrderSide side, common::Price price, common::Quantity qty = 100, common::OrderType type = common::OrderType::Limit) {
        auto order = std::make_unique<common::Order>(id, common::Instrument::EURUSD, "test_client", side, type, qty, price, std::chrono::system_clock::now());
        common::Order* ptr = order.get();
        orderStore.push_back(std::move(order));
        return ptr;
    }
};

// --- Test Cases (unchanged) ---

TEST_F(MatcherTest, NoMatch) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Sell, 101.0));
    common::Order* incomingBuy = createOrder(2, common::OrderSide::Buy, 100.0);
    auto trades = matcher->match(incomingBuy, *orderBook);
    ASSERT_TRUE(trades.empty());
}

TEST_F(MatcherTest, FullMatchSingleTrade) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Sell, 100.0, 100));
    common::Order* incomingBuy = createOrder(2, common::OrderSide::Buy, 100.0, 100);
    auto trades = matcher->match(incomingBuy, *orderBook);
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getQuantity(), 100);
    EXPECT_EQ(trades[0].getPrice(), 100.0);
    EXPECT_EQ(incomingBuy->getStatus(), common::OrderStatus::Filled);
    EXPECT_EQ(orderStore[0]->getStatus(), common::OrderStatus::Filled);
    EXPECT_TRUE(orderBook->getAskMap()->empty());
}

TEST_F(MatcherTest, PartialMatch) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Sell, 100.0, 50));
    common::Order* incomingBuy = createOrder(2, common::OrderSide::Buy, 100.0, 100);
    auto trades = matcher->match(incomingBuy, *orderBook);
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getQuantity(), 50);
    EXPECT_EQ(incomingBuy->getStatus(), common::OrderStatus::PartiallyFilled);
    EXPECT_EQ(incomingBuy->getRemainingQuantity(), 50);
    EXPECT_EQ(orderStore[0]->getStatus(), common::OrderStatus::Filled);
    EXPECT_TRUE(orderBook->getAskMap()->empty());
}

TEST_F(MatcherTest, MultipleMatches) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Sell, 100.0, 50));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Sell, 101.0, 50));
    common::Order* incomingBuy = createOrder(3, common::OrderSide::Buy, 101.0, 100);
    auto trades = matcher->match(incomingBuy, *orderBook);
    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].getQuantity(), 50);
    EXPECT_EQ(trades[0].getPrice(), 100.0);
    EXPECT_EQ(trades[1].getQuantity(), 50);
    EXPECT_EQ(trades[1].getPrice(), 101.0);
    EXPECT_EQ(incomingBuy->getStatus(), common::OrderStatus::Filled);
    EXPECT_TRUE(orderBook->getAskMap()->empty());
}

TEST_F(MatcherTest, MarketOrderPartialFill) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Sell, 100.0, 70));
    common::Order* incomingMarketBuy = createOrder(2, common::OrderSide::Buy, 0.0, 100, common::OrderType::Market);
    auto trades = matcher->match(incomingMarketBuy, *orderBook);
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getQuantity(), 70);
    EXPECT_EQ(incomingMarketBuy->getStatus(), common::OrderStatus::PartiallyFilled);
    EXPECT_EQ(incomingMarketBuy->getRemainingQuantity(), 30);
    EXPECT_TRUE(orderBook->getAskMap()->empty());
}

TEST_F(MatcherTest, MarketOrderNoFill) {
    common::Order* incomingMarketBuy = createOrder(1, common::OrderSide::Buy, 0.0, 100, common::OrderType::Market);
    auto trades = matcher->match(incomingMarketBuy, *orderBook);
    ASSERT_TRUE(trades.empty());
    EXPECT_EQ(incomingMarketBuy->getStatus(), common::OrderStatus::Cancelled);
}

TEST_F(MatcherTest, TradePriceIsCorrect) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Sell, 100.0, 100));
    common::Order* incomingBuy = createOrder(2, common::OrderSide::Buy, 105.0, 100);
    auto trades = matcher->match(incomingBuy, *orderBook);
    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getPrice(), 100.0);
}
