//
// Created by sujal on 27-10-2025.
//


#include "gtest/gtest.h"
#include "data/TradeIDRepository.h"
#include "common/Trade.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include <string>
#include <cstdio>
#include <memory>

class TradeIDRepositoryTest : public testing::Test {
protected:
    void SetUp() override {
        std::remove(pmTestDbPath.c_str());
    }

    void TearDown() override {
        std::remove(pmTestDbPath.c_str());
    }

protected:
    const std::string pmTestDbPath = "test_trade_id_repo.sqlite";
};

TEST_F(TradeIDRepositoryTest, InitializesToZero) {
    data::TradeIDRepository repo(pmTestDbPath);
    EXPECT_EQ(repo.getCurrentTradeID(), 0);
}

TEST_F(TradeIDRepositoryTest, SetCurrentTradeID_UpdatesValue) {
    common::TradeID newID = 100;
    {
        data::TradeIDRepository repo(pmTestDbPath);
        repo.setCurrentTradeID(newID);
    }

    data::TradeIDRepository readRepo(pmTestDbPath);
    EXPECT_EQ(readRepo.getCurrentTradeID(), newID);
}

TEST_F(TradeIDRepositoryTest, SetCurrentTradeID_DoesNotUpdateToLowerValue) {
    common::TradeID initialID = 100;
    common::TradeID lowerID = 50;

    {
        data::TradeIDRepository repo(pmTestDbPath);
        repo.setCurrentTradeID(initialID);
    }

    {
        data::TradeIDRepository repo(pmTestDbPath);
        repo.setCurrentTradeID(lowerID);
    }

    data::TradeIDRepository readRepo(pmTestDbPath);
    EXPECT_EQ(readRepo.getCurrentTradeID(), initialID);
}

TEST_F(TradeIDRepositoryTest, TruncateTradeID_ResetsToZero) {
    {
        data::TradeIDRepository repo(pmTestDbPath);
        repo.setCurrentTradeID(12345);
    }

    {
        data::TradeIDRepository repo(pmTestDbPath);
        EXPECT_EQ(repo.getCurrentTradeID(), 12345);
        repo.truncateTradeID();
    }

    data::TradeIDRepository readRepo(pmTestDbPath);
    EXPECT_EQ(readRepo.getCurrentTradeID(), 0);
}
