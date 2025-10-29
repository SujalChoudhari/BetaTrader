//
// Created by sujal on 29-10-2025.
//

#include "gtest/gtest.h"
#include "logging/Logger.h"
#include "trading_core/RiskManager.h"
#include "data/Constant.h"
#include "trading_core/TradeIDGenerator.h"

using namespace trading_core;

class RiskManagerTest : public testing::Test {
protected:
    void SetUp() override {
        logging::Logger::Init("risk_manager_test", "logs/risk_manager_test.log");
        data::DatabaseWorkerPtr dbWorker = std::make_shared<data::DatabaseWorker>(data::databasePath);
        tradeIDGenerator = std::make_unique<TradeIDGenerator>(dbWorker);
        riskManager = std::make_unique<RiskManager>(dbWorker);
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
            common::Instrument::EURUSD,
            "CLIENT_" + std::to_string(id),
            side,
            type,
            quantity,
            price,
            std::chrono::steady_clock::now()
        );
        order.setStatus(status);
        order.setRemainingQuantity(remainingQuantity);
        return order;
    }

    [[nodiscard]] common::Trade createTrade(
        common::OrderID buyOrderId,
        common::OrderID sellOrderId,
        common::Quantity quantity = 100.0,
        double price = 50.0
    ) const {
        return {
            tradeIDGenerator->nextId(),
            common::Instrument::EURUSD,
            buyOrderId,
            sellOrderId,
            quantity,
            price,
            std::chrono::steady_clock::now()
        };
    }

protected:
    std::unique_ptr<RiskManager> riskManager;
    std::unique_ptr<TradeIDGenerator> tradeIDGenerator;
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
    const auto trade = createTrade(1, 2, 100, 50);
    EXPECT_NO_THROW(riskManager->postTradeUpdate(trade));
}

TEST_F(RiskManagerTest, PostTradeUpdateMultipleTrades) {
    std::vector<common::Trade> trades;
    trades.push_back(createTrade(1, 2, 100, 50));
    trades.push_back(createTrade(3, 4, 150, 51));
    trades.push_back(createTrade(5, 6, 200, 49));

    EXPECT_NO_THROW(riskManager->postTradeUpdate(trades));
}

TEST_F(RiskManagerTest, PostTradeUpdateEmptyVector) {
    std::vector<common::Trade> trades;
    EXPECT_NO_THROW(riskManager->postTradeUpdate(trades));
}
