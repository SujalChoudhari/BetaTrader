//
// Created by sujal on 25-10-2025.
//

#include "data/Constant.h"
#include "gtest/gtest.h"
#include "logging/Logger.h"
#include "trading_core/Matcher.h"
#include "trading_core/OrderBook.h"

using namespace trading_core;

class MatcherTest : public testing::Test {
protected:
    void SetUp() override {
        logging::Logger::Init("matcher_test", "logs/matcher_test.log");
        pmOrderBook = std::make_unique<OrderBook>();
        data::DatabaseWorkerPtr dbWorker = std::make_shared<data::DatabaseWorker>(data::databasePath);
        matcher = std::make_unique<Matcher>(dbWorker);
    }

    void TearDown() override {
        matcher.reset();
        pmOrderBook.reset();
        logging::Logger::Shutdown();
    }

    static std::shared_ptr<common::Order> createOrder(
        common::OrderID id,
        common::OrderSide side = common::OrderSide::Buy,
        common::OrderType type = common::OrderType::Limit,
        double quantity = 100.0,
        double price = 50.0
    ) {
        return std::make_shared<common::Order>(
            id,
            common::Instrument::EURUSD,
            "CLIENT_" + std::to_string(id),
            side,
            type,
            quantity,
            price,
            std::chrono::system_clock::now()
        );
    }

protected:
    std::unique_ptr<OrderBook> pmOrderBook;
    std::unique_ptr<Matcher> matcher;
};

TEST_F(MatcherTest, NoTradeWhenEmptyBook) {
    auto order = *createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 50);
    const auto trades = matcher->match(order, *pmOrderBook);
    EXPECT_TRUE(trades.empty());
}

TEST_F(MatcherTest, TradeWhenCrossingOrders) {
    const auto buyOrder = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 50);
    const auto sellOrder = createOrder(2, common::OrderSide::Sell, common::OrderType::Limit, 100, 49);

    pmOrderBook->insertOrder(buyOrder.get());

    const auto trades = matcher->match(*sellOrder, *pmOrderBook);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getBuyOrderId(), buyOrder->getId());
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrder->getId());
    EXPECT_EQ(trades[0].getPrice(), 50);
    EXPECT_EQ(trades[0].getQuantity(), 100);
}

TEST_F(MatcherTest, NoTradeWhenNonCrossingOrders) {
    const auto buyOrder = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 48);
    const auto sellOrder = createOrder(2, common::OrderSide::Sell, common::OrderType::Limit, 100, 50);

    pmOrderBook->insertOrder(buyOrder.get());
    const auto trades = matcher->match(*sellOrder, *pmOrderBook);

    EXPECT_TRUE(trades.empty());
}

TEST_F(MatcherTest, TradeWhenMarketBidOrders) {
    const auto sellOrder1 = createOrder(1, common::OrderSide::Sell, common::OrderType::Limit, 30, 50);
    const auto sellOrder2 = createOrder(2, common::OrderSide::Sell, common::OrderType::Limit, 70, 60);
    const auto buy = createOrder(3, common::OrderSide::Buy, common::OrderType::Market, 100, 50);

    pmOrderBook->insertOrder(sellOrder1.get());
    pmOrderBook->insertOrder(sellOrder2.get());

    const auto trades = matcher->match(*buy, *pmOrderBook);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrder1->getId());
    EXPECT_EQ(trades[1].getSellOrderId(), sellOrder2->getId());
    EXPECT_EQ(trades[0].getBuyOrderId(), buy->getId());
    EXPECT_EQ(trades[0].getQuantity(), 30);
    EXPECT_EQ(trades[1].getQuantity(), 70);

    EXPECT_EQ(buy->getRemainingQuantity(), 0);
    EXPECT_EQ(buy->getStatus(), common::OrderStatus::Filled);
}

TEST_F(MatcherTest, NoTradeWhenMarketBidOrders) {
    const auto sellOrder1 = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 30, 50);
    const auto sellOrder2 = createOrder(2, common::OrderSide::Buy, common::OrderType::Limit, 70, 60);
    const auto buy = createOrder(3, common::OrderSide::Buy, common::OrderType::Market, 100, 50);

    pmOrderBook->insertOrder(sellOrder1.get());
    pmOrderBook->insertOrder(sellOrder2.get());

    const auto trades = matcher->match(*buy, *pmOrderBook);

    ASSERT_EQ(trades.size(), 0);

    EXPECT_EQ(buy->getStatus(), common::OrderStatus::New);
}


TEST_F(MatcherTest, PartialTradeWhenMarketBidOrders) {
    const auto sellOrder1 = createOrder(1, common::OrderSide::Sell, common::OrderType::Limit, 30, 50);
    const auto sellOrder2 = createOrder(2, common::OrderSide::Sell, common::OrderType::Limit, 50, 60);
    const auto buy = createOrder(3, common::OrderSide::Buy, common::OrderType::Market, 100, 50);

    pmOrderBook->insertOrder(sellOrder1.get());
    pmOrderBook->insertOrder(sellOrder2.get());

    const auto trades = matcher->match(*buy, *pmOrderBook);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrder1->getId());
    EXPECT_EQ(trades[1].getSellOrderId(), sellOrder2->getId());
    EXPECT_EQ(trades[0].getBuyOrderId(), buy->getId());
    EXPECT_EQ(trades[0].getQuantity(), 30);
    EXPECT_EQ(trades[1].getQuantity(), 50);

    EXPECT_EQ(buy->getRemainingQuantity(), 20);
    EXPECT_EQ(buy->getStatus(), common::OrderStatus::PartiallyFilled);
}


TEST_F(MatcherTest, TradeWhenMarketAskOrders) {
    const auto buyOrder1 = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 20, 10);
    const auto buyOrder2 = createOrder(2, common::OrderSide::Buy, common::OrderType::Limit, 50, 50);
    const auto sellOrder = createOrder(3, common::OrderSide::Sell, common::OrderType::Market, 70, 50);

    pmOrderBook->insertOrder(buyOrder1.get());
    pmOrderBook->insertOrder(buyOrder2.get());

    const auto trades = matcher->match(*sellOrder, *pmOrderBook);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].getBuyOrderId(), buyOrder2->getId()); // 2 goes first
    EXPECT_EQ(trades[1].getBuyOrderId(), buyOrder1->getId());
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrder->getId());
    EXPECT_EQ(trades[0].getQuantity(), 50);
    EXPECT_EQ(trades[1].getQuantity(), 20);
}


TEST_F(MatcherTest, PartialTradeWhenMarketAskOrders) {
    const auto buyOrder1 = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 30, 10);
    const auto buyOrder2 = createOrder(2, common::OrderSide::Buy, common::OrderType::Limit, 50, 50);
    const auto sellOrder = createOrder(3, common::OrderSide::Sell, common::OrderType::Market, 100, 50);

    pmOrderBook->insertOrder(buyOrder1.get());
    pmOrderBook->insertOrder(buyOrder2.get());

    const auto trades = matcher->match(*sellOrder, *pmOrderBook);

    ASSERT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].getBuyOrderId(), buyOrder2->getId()); // 2 goes first
    EXPECT_EQ(trades[1].getBuyOrderId(), buyOrder1->getId());
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrder->getId());
    EXPECT_EQ(trades[0].getQuantity(), 50);
    EXPECT_EQ(trades[1].getQuantity(), 30);

    EXPECT_EQ(sellOrder->getRemainingQuantity(), 20);
}
