//
// Created by sujal on 21-10-2025.
//

#include "gtest/gtest.h"
#include "trading-core/OrderManager.h"

class OrderManagerTest : public testing::Test {
protected:
    void SetUp() override {
        pmOrderManager = std::make_unique<trading_core::OrderManager>();
    }

    void TearDown() override {
        pmOrderManager.reset();
    }

    static common::OrderPtr createOrder(
        common::OrderID id,
        common::OrderSide side = common::OrderSide::Buy,
        double quantity = 100.0,
        double price = 50.0) {
        return std::make_shared<common::Order>(
            id,
            common::OrderSymbol::EURUSD,
            "CLIENT_" + std::to_string(id),
            side,
            common::OrderType::Limit,
            quantity,
            price,
            std::chrono::steady_clock::now()
        );
    }

protected:
    std::unique_ptr<trading_core::OrderManager> pmOrderManager;
};


TEST_F(OrderManagerTest, InitialStateIsEmpty) {
    EXPECT_EQ(pmOrderManager->size(), 0);
}

TEST_F(OrderManagerTest, AddOrderSuccessfully) {
    const auto order = createOrder(101);
    ASSERT_TRUE(pmOrderManager->addOrder(order));
    EXPECT_EQ(pmOrderManager->size(), 1);
    EXPECT_TRUE(pmOrderManager->containsOrderById(101));
}

TEST_F(OrderManagerTest, AddDuplicateOrderFails) {
    const auto order1 = createOrder(101);
    const auto order2 = createOrder(101); // New object, same ID

    ASSERT_TRUE(pmOrderManager->addOrder(order1));
    EXPECT_EQ(pmOrderManager->size(), 1);

    // Try to add the duplicate
    EXPECT_FALSE(pmOrderManager->addOrder(order2));
    EXPECT_EQ(pmOrderManager->size(), 1); // Size should not change
}

TEST_F(OrderManagerTest, AddNullOrderFails) {
    EXPECT_FALSE(pmOrderManager->addOrder(nullptr));
    EXPECT_EQ(pmOrderManager->size(), 0);
}

TEST_F(OrderManagerTest, GetExistingOrder) {
    const auto order = createOrder(101);
    pmOrderManager->addOrder(order);

    const auto retrievedOrderOpt = pmOrderManager->getOrderById(101);
    ASSERT_TRUE(retrievedOrderOpt.has_value());

    const auto& retrievedOrder = retrievedOrderOpt.value();
    EXPECT_EQ(retrievedOrder->getId(), 101);
    EXPECT_EQ(retrievedOrder, order); // Should be the same shared_ptr
}

TEST_F(OrderManagerTest, GetNonExistentOrder) {
    auto retrievedOrderOpt = pmOrderManager->getOrderById(999);
    EXPECT_FALSE(retrievedOrderOpt.has_value());
}

TEST_F(OrderManagerTest, RemoveExistingOrder) {
    const auto order = createOrder(101);
    pmOrderManager->addOrder(order);
    ASSERT_EQ(pmOrderManager->size(), 1);

    EXPECT_TRUE(pmOrderManager->removeOrderById(101));
    EXPECT_EQ(pmOrderManager->size(), 0);
    EXPECT_FALSE(pmOrderManager->containsOrderById(101));
}

TEST_F(OrderManagerTest, RemoveNonExistentOrder) {
    const auto order = createOrder(101);
    pmOrderManager->addOrder(order);
    ASSERT_EQ(pmOrderManager->size(), 1);

    EXPECT_FALSE(pmOrderManager->removeOrderById(999));
    EXPECT_EQ(pmOrderManager->size(), 1); // Size should not change
}

TEST_F(OrderManagerTest, ContainsOrder) {
    const auto order = createOrder(101);
    pmOrderManager->addOrder(order);

    EXPECT_TRUE(pmOrderManager->containsOrderById(101));
    EXPECT_FALSE(pmOrderManager->containsOrderById(999));
}

TEST_F(OrderManagerTest, SizeChangesCorrectly) {
    EXPECT_EQ(pmOrderManager->size(), 0);

    pmOrderManager->addOrder(createOrder(101));
    EXPECT_EQ(pmOrderManager->size(), 1);

    pmOrderManager->addOrder(createOrder(102, common::OrderSide::Sell));
    EXPECT_EQ(pmOrderManager->size(), 2);

    pmOrderManager->removeOrderById(101);
    EXPECT_EQ(pmOrderManager->size(), 1);

    pmOrderManager->removeOrderById(102);
    EXPECT_EQ(pmOrderManager->size(), 0);
}
