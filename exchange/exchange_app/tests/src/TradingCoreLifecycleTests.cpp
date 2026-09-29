#include "logging/Logger.h"
#include <chrono>
#include <exchange_app/TradingCore.h>
#include <exchange_routing/NewOrder.h>
#include <gtest/gtest.h>
#include <memory>

TEST(TradingCoreLifecycleTest, DestructionAfterLoggerShutdownIsSafe)
{
    logging::Logger::Init("trading_core_lifecycle_test",
                          "logs/trading_core_lifecycle_test.log", true, false);

    {
        auto tradingCore = std::make_unique<trading_core::TradingCore>();
        tradingCore->start();

        for (int i = 0; i < 10; ++i) {
            const auto orderId = tradingCore->getOrderIDGenerator()->nextId();
            auto order = std::make_unique<common::Order>(
                    orderId, orderId, common::Instrument::EURUSD,
                    "LIFECYCLE_TEST", "LIFECYCLE_TEST", common::OrderSide::Buy,
                    common::OrderType::Limit, common::TimeInForce::DAY, 1, 1.0,
                    std::chrono::system_clock::now());
            tradingCore->submitCommand(std::make_unique<trading_core::NewOrder>(
                    order->getClientId(), order->getTimestamp(),
                    std::move(order)));
        }

        tradingCore->stopAcceptingCommands();
        tradingCore->waitAllQueuesIdle();
        logging::Logger::Shutdown();
    }
}
