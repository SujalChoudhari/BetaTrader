#include "common/Order.h"
#include "data/DatabaseWorker.h"
#include "data/OrderRepository.h"
#include "data/Query.h" // To ensure create table query is available
#include "trading_core/OrderBook.h"
#include <filesystem> // For temporary file management
#include <future> // Required for std::promise and std::future
#include <gtest/gtest.h>
#include <memory>
#include <vector>

// Test Fixture for OrderBook tests
class OrderBookTest : public ::testing::Test {
protected:
    std::unique_ptr<trading_core::OrderBook> orderBook;

    // The fixture owns the orders, because the OrderBook only stores raw
    // pointers.
    std::vector<std::unique_ptr<common::Order>> orderStore;

    void SetUp() override
    {
        orderBook = std::make_unique<trading_core::OrderBook>();
    }

    // Helper to create and store an order, returning a raw pointer for
    // insertion
    common::Order* createOrder(common::OrderID id, common::OrderSide side,
                               common::Price price, common::Quantity qty = 100,
                               common::OrderStatus status
                               = common::OrderStatus::New)
    {
        auto order = std::make_unique<common::Order>(
                id, common::Instrument::EURUSD, "test_client", side,
                common::OrderType::Limit, common::TimeInForce::DAY, qty, price,
                std::chrono::system_clock::now());
        order->setStatus(status);
        if (status == common::OrderStatus::PartiallyFilled) {
            order->setRemainingQuantity(qty / 2);
        }
        common::Order* ptr = order.get();
        orderStore.push_back(std::move(order));
        return ptr;
    }
};

// --- insertOrder Tests ---

TEST_F(OrderBookTest, InsertSingleBuyOrder)
{
    common::Order* buyOrder = createOrder(1, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder);

    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    ASSERT_EQ(orderBook->getAskMap()->size(), 0);
    ASSERT_EQ(orderBook->getBidMap()->at(100.0).size(), 1);
    EXPECT_EQ(orderBook->getBidMap()->at(100.0).front()->getClientOrderId(), 1);
}

TEST_F(OrderBookTest, InsertMultipleOrdersAtSamePrice)
{
    common::Order* buyOrder1 = createOrder(1, common::OrderSide::Buy, 100.0);
    common::Order* buyOrder2 = createOrder(2, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder1);
    orderBook->insertOrder(buyOrder2);

    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    const auto& priceLevel = orderBook->getBidMap()->at(100.0);
    ASSERT_EQ(priceLevel.size(), 2);
    EXPECT_EQ(priceLevel[0]->getClientOrderId(), 1); // Check time priority
    EXPECT_EQ(priceLevel[1]->getClientOrderId(), 2);
}

TEST_F(OrderBookTest, InsertMultipleOrdersAtDifferentPrices)
{
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

TEST_F(OrderBookTest, CancelExistingOrder)
{
    common::Order* buyOrder = createOrder(1, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder);
    ASSERT_EQ(orderBook->getBidMap()->size(), 1);

    bool result = orderBook->cancelOrder(1);
    ASSERT_TRUE(result);
    EXPECT_EQ(orderBook->getBidMap()->size(), 0);
}

TEST_F(OrderBookTest, CancelNonExistentOrder)
{
    common::Order* buyOrder = createOrder(1, common::OrderSide::Buy, 100.0);
    orderBook->insertOrder(buyOrder);

    bool result = orderBook->cancelOrder(999); // ID does not exist
    ASSERT_FALSE(result);
    EXPECT_EQ(orderBook->getBidMap()->size(), 1);
}

TEST_F(OrderBookTest, CancelOrderEmptiesPriceLevel)
{
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 100.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 101.0));
    ASSERT_EQ(orderBook->getBidMap()->size(), 2);

    orderBook->cancelOrder(2);
    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    EXPECT_TRUE(orderBook->getBidMap()->count(100.0));
    EXPECT_FALSE(orderBook->getBidMap()->count(101.0));
}

TEST_F(OrderBookTest, CancelOrderLeavesPriceLevel)
{
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 100.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 100.0));
    ASSERT_EQ(orderBook->getBidMap()->at(100.0).size(), 2);

    orderBook->cancelOrder(1);
    ASSERT_EQ(orderBook->getBidMap()->size(), 1);
    const auto& priceLevel = orderBook->getBidMap()->at(100.0);
    ASSERT_EQ(priceLevel.size(), 1);
    EXPECT_EQ(priceLevel.front()->getClientOrderId(), 2);
}

// This test is designed to fail if the iterator invalidation bug exists.
TEST_F(OrderBookTest, CancelOrderIteratorInvalidationBug)
{
    // Insert two orders at adjacent price levels.
    orderBook->insertOrder(createOrder(1, common::OrderSide::Buy, 101.0));
    orderBook->insertOrder(createOrder(2, common::OrderSide::Buy, 100.0));
    ASSERT_EQ(orderBook->getBidMap()->size(), 2);

    // Cancel the first element (price 101.0).
    // The buggy implementation will erase this iterator, then the loop will
    // increment, skipping the next element (price 100.0).
    bool result1 = orderBook->cancelOrder(1);
    ASSERT_TRUE(result1);

    // Now, try to cancel the second element.
    // With the bug, this element was skipped and will not be found.
    // A correct implementation would find and cancel it.
    bool result2 = orderBook->cancelOrder(2);
    ASSERT_TRUE(result2) << "The cancelOrder implementation likely has an "
                            "iterator invalidation bug.";
    EXPECT_EQ(orderBook->getBidMap()->size(), 0);
}

// Test Fixture for Persistence-related OrderBook tests
class OrderBookPersistenceTest : public ::testing::Test {
protected:
    std::string dbPath = "test_order_persistence.db";
    std::unique_ptr<data::DatabaseWorker> dbWorker;
    std::unique_ptr<data::OrderRepository> orderRepository;
    std::vector<std::unique_ptr<common::Order>> orderStore;

    void SetUp() override
    {
        // Clean up any previous test database
        std::filesystem::remove(dbPath);

        dbWorker = std::make_unique<data::DatabaseWorker>(dbPath);
        orderRepository
                = std::make_unique<data::OrderRepository>(dbWorker.get());

        // Ensure database is initialized before proceeding
        std::promise<void> db_init_promise;
        std::future<void> db_init_future = db_init_promise.get_future();
        dbWorker->enqueue([&db_init_promise](SQLite::Database& db) {
            db_init_promise.set_value();
        });
        db_init_future.wait();
    }

    void TearDown() override
    {
        // Ensure all pending DB operations are complete before closing
        std::promise<void> db_shutdown_promise;
        std::future<void> db_shutdown_future = db_shutdown_promise.get_future();
        dbWorker->enqueue([&db_shutdown_promise](SQLite::Database& db) {
            db_shutdown_promise.set_value();
        });
        db_shutdown_future.wait();

        dbWorker.reset();
        std::filesystem::remove(dbPath);
    }

    // Helper to create and store an order
    common::Order* createAndStoreOrder(common::OrderID id,
                                       common::OrderSide side,
                                       common::Price price,
                                       common::Quantity qty,
                                       common::OrderStatus status)
    {
        auto order = std::make_unique<common::Order>(
                id, common::Instrument::EURUSD, "client1", side,
                common::OrderType::Limit, common::TimeInForce::DAY, qty, price,
                std::chrono::system_clock::now());
        order->setStatus(status);
        if (status == common::OrderStatus::PartiallyFilled) {
            order->setRemainingQuantity(qty / 2);
        }
        common::Order* ptr = order.get();
        orderStore.push_back(std::move(order));
        return ptr;
    }
};

TEST_F(OrderBookPersistenceTest, LoadsOnlyActiveOrdersIntoOrderBook)
{
    // 1. Save a mix of active and inactive orders
    common::Order* newOrder = createAndStoreOrder(
            1, common::OrderSide::Buy, 100.0, 100, common::OrderStatus::New);
    common::Order* partiallyFilledOrder
            = createAndStoreOrder(2, common::OrderSide::Sell, 101.0, 200,
                                  common::OrderStatus::PartiallyFilled);
    common::Order* filledOrder = createAndStoreOrder(
            3, common::OrderSide::Buy, 99.0, 50, common::OrderStatus::Filled);
    common::Order* cancelledOrder
            = createAndStoreOrder(4, common::OrderSide::Sell, 102.0, 150,
                                  common::OrderStatus::Cancelled);
    common::Order* anotherNewOrder = createAndStoreOrder(
            5, common::OrderSide::Buy, 100.0, 75, common::OrderStatus::New);

    orderRepository->saveOrder(*newOrder);
    orderRepository->saveOrder(*partiallyFilledOrder);
    orderRepository->saveOrder(*filledOrder);
    orderRepository->saveOrder(*cancelledOrder);
    orderRepository->saveOrder(*anotherNewOrder);

    // Ensure all saves are processed by the DatabaseWorker
    std::promise<void> save_promise;
    std::future<void> save_future = save_promise.get_future();
    dbWorker->enqueue([&save_promise](SQLite::Database& db) {
        save_promise.set_value();
    });
    save_future.wait();

    // 2. Simulate restart: Create a new OrderBook and load orders
    trading_core::OrderBook newOrderBook;
    std::vector<common::Order> loadedOrders;
    std::promise<void> load_promise;
    std::future<void> load_future = load_promise.get_future();

    orderRepository->loadOrdersForInstrument(
            common::Instrument::EURUSD,
            [&loadedOrders, &load_promise](std::vector<common::Order> orders) {
                loadedOrders = std::move(orders);
                load_promise.set_value();
            });
    load_future.wait();

    // 3. Populate the new OrderBook with loaded orders
    // We need to manage the lifetime of these loaded orders for the OrderBook
    std::vector<std::unique_ptr<common::Order>> activeOrdersForBook;
    for (auto& order: loadedOrders) {
        activeOrdersForBook.push_back(std::make_unique<common::Order>(order));
        newOrderBook.insertOrder(activeOrdersForBook.back().get());
    }

    // 4. Verify the state of the new OrderBook
    ASSERT_EQ(loadedOrders.size(),
              3); // Only New and PartiallyFilled orders should be loaded

    // Check BidMap (Buy orders)
    ASSERT_EQ(newOrderBook.getBidMap()->size(), 1);
    ASSERT_TRUE(newOrderBook.getBidMap()->count(100.0));
    ASSERT_EQ(newOrderBook.getBidMap()->at(100.0).size(),
              2); // Order 1 and Order 5
    EXPECT_EQ(newOrderBook.getBidMap()->at(100.0)[0]->getClientOrderId(), 1);
    EXPECT_EQ(newOrderBook.getBidMap()->at(100.0)[1]->getClientOrderId(), 5);

    // Check AskMap (Sell orders)
    ASSERT_EQ(newOrderBook.getAskMap()->size(), 1);
    ASSERT_TRUE(newOrderBook.getAskMap()->count(101.0));
    ASSERT_EQ(newOrderBook.getAskMap()->at(101.0).size(), 1); // Order 2
    EXPECT_EQ(newOrderBook.getAskMap()->at(101.0)[0]->getClientOrderId(), 2);

    // Ensure inactive orders are NOT in the book
    ASSERT_FALSE(newOrderBook.getBidMap()->count(99.0)); // Filled order
    ASSERT_FALSE(newOrderBook.getAskMap()->count(102.0)); // Cancelled order
}
