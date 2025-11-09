#include "data/OrderRepository.h"
#include "data/DatabaseWorker.h"
#include "common/Order.h"
#include <gtest/gtest.h>
#include <memory>
#include <future>

class OrderRepositoryTests : public ::testing::Test {
protected:
    void SetUp() override {
        dbWorker = std::make_unique<data::DatabaseWorker>(":memory:");
        orderRepository = std::make_unique<data::OrderRepository>(dbWorker.get());
    }

    void TearDown() override {
        dbWorker.reset();
    }

    std::unique_ptr<data::DatabaseWorker> dbWorker;
    std::unique_ptr<data::OrderRepository> orderRepository;
};

TEST_F(OrderRepositoryTests, SaveAndLoadOrder) {
    const common::Order order(1, common::Instrument::EURUSD, "client1", common::OrderSide::Buy,
                              common::OrderType::Limit, common::TimeInForce::DAY, 100, 1.1, std::chrono::system_clock::now());

    orderRepository->saveOrder(order);

    std::promise<std::vector<common::Order> > promise;
    std::future<std::vector<common::Order> > future = promise.get_future();

    orderRepository->loadOrdersForInstrument(common::Instrument::EURUSD, [&promise](std::vector<common::Order> orders) {
        promise.set_value(orders);
    });

    std::vector<common::Order> loadedOrders = future.get();

    ASSERT_EQ(loadedOrders.size(), 1);
    EXPECT_EQ(loadedOrders[0].getId(), order.getId());
    EXPECT_EQ(loadedOrders[0].getClientId(), order.getClientId());
    EXPECT_EQ(loadedOrders[0].getSymbol(), order.getSymbol());
    EXPECT_EQ(loadedOrders[0].getSide(), order.getSide());
    EXPECT_EQ(loadedOrders[0].getOrderType(), order.getOrderType());
    EXPECT_EQ(loadedOrders[0].getPrice(), order.getPrice());
    EXPECT_EQ(loadedOrders[0].getOriginalQuantity(), order.getOriginalQuantity());
    EXPECT_EQ(loadedOrders[0].getRemainingQuantity(), order.getRemainingQuantity());
    EXPECT_EQ(loadedOrders[0].getStatus(), order.getStatus());
}

TEST_F(OrderRepositoryTests, UpdateOrder) {
    common::Order order(1, common::Instrument::EURUSD, "client1", common::OrderSide::Buy, common::OrderType::Limit, common::TimeInForce::DAY, 100,
                        1.1, std::chrono::system_clock::now());
    orderRepository->saveOrder(order);

    order.setRemainingQuantity(50);
    order.setStatus(common::OrderStatus::PartiallyFilled);
    orderRepository->updateOrder(order);

    std::promise<std::vector<common::Order> > promise;
    std::future<std::vector<common::Order> > future = promise.get_future();

    orderRepository->loadOrdersForInstrument(common::Instrument::EURUSD, [&promise](std::vector<common::Order> orders) {
        promise.set_value(orders);
    });

    std::vector<common::Order> loadedOrders = future.get();

    ASSERT_EQ(loadedOrders.size(), 1);
    EXPECT_EQ(loadedOrders[0].getRemainingQuantity(), 50);
    EXPECT_EQ(loadedOrders[0].getStatus(), common::OrderStatus::PartiallyFilled);
}

TEST_F(OrderRepositoryTests, RemoveOrder) {
    const common::Order order(1, common::Instrument::EURUSD, "client1", common::OrderSide::Buy,
                              common::OrderType::Limit, common::TimeInForce::DAY, 100, 1.1, std::chrono::system_clock::now());
    orderRepository->saveOrder(order);

    orderRepository->removeOrder(order.getId());

    std::promise<std::vector<common::Order> > promise;
    std::future<std::vector<common::Order> > future = promise.get_future();

    orderRepository->loadOrdersForInstrument(common::Instrument::EURUSD, [&promise](std::vector<common::Order> orders) {
        promise.set_value(orders);
    });

    std::vector<common::Order> loadedOrders = future.get();

    ASSERT_EQ(loadedOrders.size(), 0);
}
