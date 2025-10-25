//
// Created by sujal on 25-10-2025.
//

#include "gtest/gtest.h"
#include "trading-core/Matcher.h"
#include "trading-core/OrderBook.h"
#include "trading-core/Order.h"
#include "trading-core/Trade.h"

using namespace trading_core;

class MatcherTest : public testing::Test {
protected:
    void SetUp() override {
        pmOrderBook = std::make_unique<OrderBook>();
        matcher = std::make_unique<Matcher>();
    }

    void TearDown() override {
        matcher.reset();
        pmOrderBook.reset();
    }

    static OrderPtr createOrder(
        common::OrderID id,
        common::OrderSide side = common::OrderSide::Buy,
        double quantity = 100.0,
        double price = 50.0
    ) {
        return std::make_shared<Order>(
            id,
            "TEST_TICKER",
            "CLIENT_" + std::to_string(id),
            side,
            common::OrderType::Limit,
            quantity,
            price,
            std::chrono::steady_clock::now()
        );
    }

protected:
    std::unique_ptr<OrderBook> pmOrderBook;
    std::unique_ptr<Matcher> matcher;
};

TEST_F(MatcherTest, NoTradeWhenEmptyBook) {
    auto order = *createOrder(1, common::OrderSide::Buy, 100, 50);
    const auto trades = matcher->match(order, *pmOrderBook);
    EXPECT_TRUE(trades.empty());
}

TEST_F(MatcherTest, TradeWhenCrossingOrders) {
    const auto buyOrder = createOrder(1, common::OrderSide::Buy, 100, 50);
    const auto sellOrder = createOrder(2, common::OrderSide::Sell, 100, 49);

    pmOrderBook->insertOrder(buyOrder); // Assume OrderBook::addOrder exists

    const auto trades = matcher->match(*sellOrder, *pmOrderBook);

    ASSERT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].getBuyOrderId(), buyOrder->getId());
    EXPECT_EQ(trades[0].getSellOrderId(), sellOrder->getId());
    EXPECT_EQ(trades[0].getPrice(), 50);
    EXPECT_EQ(trades[0].getQty(), 100);
}

TEST_F(MatcherTest, NoTradeWhenNonCrossingOrders) {
    const auto buyOrder = createOrder(1, common::OrderSide::Buy, 100, 48);
    const auto sellOrder = createOrder(2, common::OrderSide::Sell, 100, 50);

    pmOrderBook->insertOrder(buyOrder);
    const auto trades = matcher->match(*sellOrder, *pmOrderBook);

    EXPECT_TRUE(trades.empty());
}
