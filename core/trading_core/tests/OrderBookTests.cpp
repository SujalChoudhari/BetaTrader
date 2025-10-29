//
// Created by sujal on 23-10-2025.
//

#include "gtest/gtest.h"
#include "trading_core/OrderBook.h"

class OrderBookTest : public testing::Test {
protected:
    void SetUp() override {
        pmOrderBook = std::make_unique<trading_core::OrderBook>();
    }

    void TearDown() override {
        pmOrderBook.reset();
    }

    static std::shared_ptr<common::Order> createOrder(
        common::OrderID id,
        common::OrderSide side = common::OrderSide::Buy,
        double quantity = 100.0,
        double price = 50.0) {
        return std::make_shared<common::Order>(
            id,
            common::Instrument::EURUSD,
            "CLIENT_" + std::to_string(id),
            side,
            common::OrderType::Limit,
            quantity,
            price,
            std::chrono::system_clock::now()
        );
    }

protected:
    std::unique_ptr<trading_core::OrderBook> pmOrderBook;
};


TEST_F(OrderBookTest, InsertSingleBuyOrder) {
    auto order = createOrder(1, common::OrderSide::Buy, 100.0, 50.0);
    pmOrderBook->insertOrder(order.get());
    auto bids = pmOrderBook->getBidMap();
    ASSERT_EQ(bids->size(), 1);
    ASSERT_EQ(bids->begin()->second.front(), order.get());
}

TEST_F(OrderBookTest, InsertSingleSellOrder) {
    auto order = createOrder(2, common::OrderSide::Sell, 100.0, 51.0);
    pmOrderBook->insertOrder(order.get());
    auto asks = pmOrderBook->getAskMap();
    ASSERT_EQ(asks->size(), 1);
    ASSERT_EQ(asks->begin()->second.front(), order.get());
}


TEST_F(OrderBookTest, MultipleBuyOrdersDescending) {
    auto o1 = createOrder(1, common::OrderSide::Buy, 100.0, 50.0);
    auto o2 = createOrder(2, common::OrderSide::Buy, 100.0, 55.0);
    pmOrderBook->insertOrder(o1.get());
    pmOrderBook->insertOrder(o2.get());
    auto bids = pmOrderBook->getBidMap();
    ASSERT_EQ(bids->begin()->first, 55.0);
}


TEST_F(OrderBookTest, MultipleSellOrdersAscending) {
    auto o1 = createOrder(1, common::OrderSide::Sell, 100.0, 50.0);
    auto o2 = createOrder(2, common::OrderSide::Sell, 100.0, 45.0);
    pmOrderBook->insertOrder(o1.get());
    pmOrderBook->insertOrder(o2.get());
    auto asks = pmOrderBook->getAskMap();
    ASSERT_EQ(asks->begin()->first, 45.0);
}


TEST_F(OrderBookTest, FIFOWithinSamePriceLevel) {
    auto o1 = createOrder(1);
    auto o2 = createOrder(2);
    pmOrderBook->insertOrder(o1.get());
    pmOrderBook->insertOrder(o2.get());
    auto bids = pmOrderBook->getBidMap();
    auto &level = bids->begin()->second;
    ASSERT_EQ(level.front(), o1.get());
    ASSERT_EQ(level.back(), o2.get());
}

TEST_F(OrderBookTest, CancelExistingOrder) {
    auto o1 = createOrder(1);
    pmOrderBook->insertOrder(o1.get());
    bool result = pmOrderBook->cancelOrder(1);
    ASSERT_TRUE(result);
    ASSERT_TRUE(pmOrderBook->getBidMap()->empty());
}


TEST_F(OrderBookTest, CancelNonexistentOrder) {
    bool result = pmOrderBook->cancelOrder(999);
    ASSERT_FALSE(result);
}

TEST_F(OrderBookTest, MixedBuyAndSellOrders) {
    auto buy = createOrder(1, common::OrderSide::Buy, 100.0, 50.0);
    auto sell = createOrder(2, common::OrderSide::Sell, 100.0, 55.0);
    pmOrderBook->insertOrder(buy.get());
    pmOrderBook->insertOrder(sell.get());
    ASSERT_EQ(pmOrderBook->getBidMap()->size(), 1);
    ASSERT_EQ(pmOrderBook->getAskMap()->size(), 1);
}


TEST_F(OrderBookTest, CancelOneAmongMany) {
    auto o1 = createOrder(1);
    auto o2 = createOrder(2);
    pmOrderBook->insertOrder(o1.get());
    pmOrderBook->insertOrder(o2.get());
    pmOrderBook->cancelOrder(1);
    auto bids = pmOrderBook->getBidMap();
    ASSERT_EQ(bids->begin()->second.size(), 1);
    ASSERT_EQ(bids->begin()->second.front(), o2.get());
}


TEST_F(OrderBookTest, AllOrdersCancelledRemovesLevels) {
    auto o1 = createOrder(1, common::OrderSide::Buy, 100.0, 50.0);
    auto o2 = createOrder(2, common::OrderSide::Buy, 100.0, 55.0);
    pmOrderBook->insertOrder(o1.get());
    pmOrderBook->insertOrder(o2.get());
    pmOrderBook->cancelOrder(1);
    pmOrderBook->cancelOrder(2);
    ASSERT_TRUE(pmOrderBook->getBidMap()->empty());
}
