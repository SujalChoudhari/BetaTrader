#include <gtest/gtest.h>
#include "trading_core/OrderBook.h"
#include "common/Order.h"
#include <memory>
#include <vector>

// Test Fixture for OrderBook tests
class OrderBookTest : public ::testing::Test {
protected:
    std::unique_ptr<trading_core::OrderBook> orderBook;
    
    // The fixture owns the orders, because the OrderBook only stores raw pointers.
    std::vector<std::unique_ptr<common::Order>> orderStore;

    void SetUp() override {
        orderBook = std::make_unique<trading_core::OrderBook>();
    }

    // Helper to create and store an order, returning a raw pointer for insertion
    common::Order* createOrder(common::OrderID id, common::OrderSide side, common::Price price) {
        auto order = std::make_unique<common::Order>(id, common::Instrument::EURUSD, "test_client", side, common::OrderType::Limit, 100, price, std::chrono::system_clock::now());
        common::Order* ptr = order.get();
        orderStore.push_back(std::move(order));
        return ptr;
    }
};

// --- insertOrder Tests ---

TEST_F(OrderBookTest, InsertSingleBuyOrder) {
    common::Order* buyOrder = createOrder(1, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder);

    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    ASSERT_EQ(orderBook->getAskMap()->size(), 0);
    ASSERT_EQ(orderBook->getBidMap()->at(100.0).size(), 1);
    EXPECT_EQ(orderBook->getBidMap()->at(100.0).front()->getId(), 1);
}

TEST_F(OrderBookTest, InsertMultipleOrdersAtSamePrice) {
    common::Order* buyOrder1 = createOrder(1, common::OrderSide::Buy, 100.0);
    common::Order* buyOrder2 = createOrder(2, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder1);
    orderBook->insertOrder(buyOrder2);

    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    const auto& priceLevel = orderBook->getBidMap()->at(100.0);
    ASSERT_EQ(priceLevel.size(), 2);
    EXPECT_EQ(priceLevel[0]->getId(), 1); // Check time priority
    EXPECT_EQ(priceLevel[1]->getId(), 2);
}

TEST_F(OrderBookTest, InsertMultipleOrdersAtDifferentPrices) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 100.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 102.0));
    orderBook->insertOrder(createOrder(3, common::OrderSide::Buy, 101.0));

    ASSERT_EQ(orderBook->getBidMap()->size(), 3);
    auto it = orderBook->getBidMap()->begin();
    EXPECT_EQ(it->first, 102.0); // Bids are sorted high to low
    it++;
    EXPECT_EQ(it->first, 101.0);
    it++;
    EXPECT_EQ(it->first, 100.0);
}

// --- cancelOrder Tests ---

TEST_F(OrderBookTest, CancelExistingOrder) {
    common::Order* buyOrder = createOrder(1, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder);
    ASSERT_EQ(orderBook->getBidMap()->size(), 1);

    bool result = orderBook->cancelOrder(1);
    ASSERT_TRUE(result);
    EXPECT_EQ(orderBook->getBidMap()->size(), 0);
}

TEST_F(OrderBookTest, CancelNonExistentOrder) {
    common::Order* buyOrder = createOrder(1, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder);
    
    bool result = orderBook->cancelOrder(999); // ID does not exist
    ASSERT_FALSE(result);
    EXPECT_EQ(orderBook->getBidMap()->size(), 1);
}

TEST_F(OrderBookTest, CancelOrderEmptiesPriceLevel) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 100.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 101.0));
    ASSERT_EQ(orderBook->getBidMap()->size(), 2);

    orderBook->cancelOrder(2);
    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    EXPECT_TRUE(orderBook->getBidMap()->count(100.0));
    EXPECT_FALSE(orderBook->getBidMap()->count(101.0));
}

TEST_F(OrderBookTest, CancelOrderLeavesPriceLevel) {
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 100.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 100.0));
    ASSERT_EQ(orderBook->getBidMap()->at(100.0).size(), 2);

    orderBook->cancelOrder(1);
    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    const auto& priceLevel = orderBook->getBidMap()->at(100.0);
    ASSERT_EQ(priceLevel.size(), 1);
    EXPECT_EQ(priceLevel.front()->getId(), 2);
}

// This test is designed to fail if the iterator invalidation bug exists.
TEST_F(OrderBookTest, CancelOrderIteratorInvalidationBug) {
    // Insert two orders at adjacent price levels.
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 101.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 100.0));
    ASSERT_EQ(orderBook->getBidMap()->size(), 2);

    // Cancel the first element (price 101.0). 
    // The buggy implementation will erase this iterator, then the loop will increment,
    // skipping the next element (price 100.0).
    bool result1 = orderBook->cancelOrder(1);
    ASSERT_TRUE(result1);

    // Now, try to cancel the second element. 
    // With the bug, this element was skipped and will not be found.
    // A correct implementation would find and cancel it.
    bool result2 = orderBook->cancelOrder(2);
    ASSERT_TRUE(result2) << "The cancelOrder implementation likely has an iterator invalidation bug.";
    EXPECT_EQ(orderBook->getBidMap()->size(), 0);
}
