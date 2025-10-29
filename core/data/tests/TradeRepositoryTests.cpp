//
// Created by sujal on 27-10-2025.
//


#include "gtest/gtest.h"
#include "data/TradeRepository.h"
#include "common/Trade.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include <string>
#include <cstdio>
#include <memory>
#include <chrono>

#include "sqlite3.h"

class TradeRepositoryTest : public testing::Test {
protected:
    void SetUp() override {
        std::remove(kTestDbPath.c_str());
    }

    void TearDown() override {
        std::remove(kTestDbPath.c_str());
    }

protected:
    const std::string kTestDbPath = "test_trade_repo.sqlite";
};

TEST_F(TradeRepositoryTest, AddTrade_InsertsTradeCorrectly) {
    const auto timestamp = std::chrono::steady_clock::now();
    const common::Trade trade(
        123,
        common::OrderSymbol::USDINR,
        10,
        11,
        100,
        50.75,
        timestamp
    );

    {
        data::TradeRepository repo(kTestDbPath);
        repo.addTrade(trade);
    }

    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        timestamp.time_since_epoch()
    ).count();

    SQLite::Database db(kTestDbPath, SQLite::OPEN_READONLY);
    SQLite::Statement query(
        db, "SELECT trade_id, buy_order_id, sell_order_id, quantity, price, timestamp_ns FROM trade WHERE trade_id = ?");
    query.bind(1, static_cast<sqlite3_int64>(trade.getTradeId()));

    ASSERT_TRUE(query.executeStep());
    EXPECT_EQ(query.getColumn(0).getInt64(), trade.getTradeId());
    EXPECT_EQ(query.getColumn(1).getInt64(), trade.getBuyOrderId());
    EXPECT_EQ(query.getColumn(2).getInt64(), trade.getSellOrderId());
    EXPECT_EQ(query.getColumn(3).getInt64(), trade.getQty());
    EXPECT_EQ(query.getColumn(4).getDouble(), trade.getPrice());
    EXPECT_EQ(query.getColumn(5).getInt64(), ns);

    EXPECT_FALSE(query.executeStep());
}
