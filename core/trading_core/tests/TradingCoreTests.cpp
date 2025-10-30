//
// Created by sujal on 30-10-2025.
//

#include <gtest/gtest.h>
#include "trading_core/TradingCore.h"
#include "trading_core/NewOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/CancelOrder.h"
#include "common/Instrument.h"
#include "common/Order.h"
#include "trading_core/OrderIDGenerator.h"

using namespace trading_core;
using namespace std::chrono_literals;

class TradingCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        tradingCore = std::make_unique<TradingCore>();
        tradingCore->start();
    }

    void TearDown() override {
        tradingCore->stop();
        tradingCore.reset();
    }

    static std::shared_ptr<common::Order> createOrder(
        common::OrderSide side = common::OrderSide::Buy,
        common::OrderType type = common::OrderType::Limit,
        double quantity = 100.0,
        double price = 50.0
    ) {
        return std::make_shared<common::Order>(
            OrderIDGenerator::nextId(),
            common::Instrument::EURUSD,
            "CLIENT_" + std::to_string(OrderIDGenerator::getId()),
            side,
            type,
            quantity,
            price,
            std::chrono::system_clock::now()
        );
    }

    std::unique_ptr<TradingCore> tradingCore;
};

TEST_F(TradingCoreTest, SubmitNewOrder) {
    const auto order = createOrder();
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);

    EXPECT_NO_THROW(tradingCore->submitCommand(std::move(newOrder)));
}

TEST_F(TradingCoreTest, SubmitModifyOrder) {
    const auto order = createOrder();
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
    tradingCore->submitCommand(std::move(newOrder));

    std::this_thread::sleep_for(100ms);

    auto modifyOrder = std::make_unique<ModifyOrder>(std::chrono::system_clock::now(), order->getClientId(),
                                                     order->getId(), 80.0, 52.0);
    EXPECT_NO_THROW(tradingCore->submitCommand(std::move(modifyOrder)));
}

TEST_F(TradingCoreTest, SubmitCancelOrder) {
    const auto order = createOrder();
    auto newOrder = std::make_unique<NewOrder>(order->getClientId(), std::chrono::system_clock::now(), order);
    tradingCore->submitCommand(std::move(newOrder));

    std::this_thread::sleep_for(100ms);

    auto cancelOrder = std::make_unique<CancelOrder>(order->getClientId(), std::chrono::system_clock::now(),
                                                     order->getId());
    EXPECT_NO_THROW(tradingCore->submitCommand(std::move(cancelOrder)));
}

TEST_F(TradingCoreTest, SubmitInvalidCommand) {
    class InvalidCommand : public Command {
    public:
        InvalidCommand() : Command(static_cast<CommandType>(99), "invalid_client", std::chrono::system_clock::now()) {
        }
    };

    auto invalidCommand = std::make_unique<InvalidCommand>();
    EXPECT_THROW(tradingCore->submitCommand(std::move(invalidCommand)), std::runtime_error);
}
