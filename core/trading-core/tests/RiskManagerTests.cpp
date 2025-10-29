//
// Created by sujal on 29-10-2025.
//

#include "gtest/gtest.h"
#include "logging/Logger.h"
#include "trading-core/RiskManager.h"
#include "data/Constant.h"
#include "trading-core/TradeIDGenerator.h"

using namespace trading_core;

class RiskManagerTest : public testing::Test {
protected:
    void SetUp() override {
        logging::Logger::Init("risk_manager_test", "logs/risk_manager_test.log");
        riskManager = std::make_unique<RiskManager>();
    }

    void TearDown() override {
        riskManager.reset();
        logging::Logger::Shutdown();
    }

    static common::Order createOrder(
        common::OrderID id,
        common::OrderSide side = common::OrderSide::Buy,
        common::OrderType type = common::OrderType::Limit,
        double quantity = 100.0,
        double remainingQuantity = 100.0,
        double price = 50.0,
        common::OrderStatus status = common::OrderStatus::New
    ) {
        common::Order order(
            id,
            common::OrderSymbol::EURUSD,
            "CLIENT_" + std::to_string(id),
            side,
            type,
            quantity,
            price,
            std::chrono::steady_clock::now()
        );
        order.setStatus(status);
        order.setRemainingQty(remainingQuantity);
        return order;
    }

    static common::Trade createTrade(
        common::OrderID buyOrderId,
        common::OrderID sellOrderId,
        double price = 50.0,
        double quantity = 100.0
    ) {
        return common::Trade(
            TradeIDGenerator::nextId(),
            buyOrderId,
            sellOrderId,
            quantity,
            price,
            std::chrono::steady_clock::now()
        );
    }

protected:
    std::unique_ptr<RiskManager> riskManager;
};

TEST_F(RiskManagerTest, PreCheckValidLimitOrder) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 100, 50);
    EXPECT_TRUE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckValidMarketOrder) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Market, 100, 100, 0);
    EXPECT_TRUE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsForInvalidOrderId) {
    const auto order = createOrder(0, common::OrderSide::Buy, common::OrderType::Limit, 100, 100, 50);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsForZeroQuantity) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 0, 0, 50);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsForNegativeQuantity) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, -100, -100, 50);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsWhenRemainingNotEqualOriginal) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 50, 50);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsForNonNewStatus) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 100, 50,
                                   common::OrderStatus::Filled);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsForLimitOrderWithZeroPrice) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 100, 0);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PreCheckFailsForLimitOrderWithNegativePrice) {
    const auto order = createOrder(1, common::OrderSide::Buy, common::OrderType::Limit, 100, 100, -50);
    EXPECT_FALSE(riskManager->preCheck(order));
}

TEST_F(RiskManagerTest, PostTradeUpdateSingleTrade) {
    const auto trade = createTrade(1, 2, 50, 100);
    EXPECT_NO_THROW(riskManager->postTradeUpdate(trade));
}

TEST_F(RiskManagerTest, PostTradeUpdateMultipleTrades) {
    std::vector<common::Trade> trades;
    trades.push_back(createTrade(1, 2, 50, 100));
    trades.push_back(createTrade(3, 4, 51, 150));
    trades.push_back(createTrade(5, 6, 49, 200));

    EXPECT_NO_THROW(riskManager->postTradeUpdate(trades));
}

TEST_F(RiskManagerTest, PostTradeUpdateEmptyVector) {
    std::vector<common::Trade> trades;
    EXPECT_NO_THROW(riskManager->postTradeUpdate(trades));
}
