//
// Created by sujal on 30-10-2025.
//

#include <gtest/gtest.h>
#include "trading_core/TradingCore.h"
#include "trading_core/Command.h"
#include "common/Instrument.h"
#include "common/Order.h"

using namespace trading_core;

class TradingCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        tradingCore = std::make_unique<TradingCore>();
    }

    void TearDown() override {
        tradingCore.reset();
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

    std::unique_ptr<TradingCore> tradingCore;
};

TEST_F(TradingCoreTest, ConstructorDoesNotThrow) {
    EXPECT_NO_THROW({
        TradingCore core;
        });
}

TEST_F(TradingCoreTest, StartStopDoNotThrow) {
    EXPECT_NO_THROW({
        tradingCore->start();
        tradingCore->stop();
        });
}

TEST_F(TradingCoreTest, NewOrderNoThrow) {
    EXPECT_NO_THROW({
        const auto order = createOrder(101);
        auto newOrder = NewOrder("101",std::chrono::system_clock::now(), *order.get());

        tradingCore->start();

        tradingCore->submitCommand(&newOrder);
        tradingCore->stop();
        });
}


TEST_F(TradingCoreTest, ModifyOrderNoThrow) {
    tradingCore->start();
    EXPECT_NO_THROW({
        const auto order = createOrder(101,common::OrderSide::Buy,common::OrderType::Limit,100,40);
        auto newOrder = NewOrder("101",std::chrono::system_clock::now(), *order.get());

        tradingCore->submitCommand(&newOrder);

        });
    std::this_thread::sleep_for(std::chrono_literals::operator ""s(1));
    EXPECT_NO_THROW({
        auto modifyOrder = ModifyOrder(std::chrono::system_clock::now(),"101",101,30,40);
        tradingCore->submitCommand(&modifyOrder);
        });
    tradingCore->stop();
}
