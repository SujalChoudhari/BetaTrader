//
// Created by sujal on 27-10-2025.
//

#include "gtest/gtest.h"
#include "data/TradeIDRepository.h"
#include "data/DatabaseWorker.h"
#include "common/Trade.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include <string>
#include <cstdio>
#include <memory>
#include <thread>

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
    data::DatabaseWorker dbWorker(pmTestDbPath);
    data::TradeIDRepository repo(dbWorker);
    EXPECT_EQ(repo.getCurrentTradeID(), 0);
}

TEST_F(TradeIDRepositoryTest, SetCurrentTradeID_UpdatesValue) {
    common::TradeID newID = 100;

    {
        data::DatabaseWorker dbWorker(pmTestDbPath);
        data::TradeIDRepository repo(dbWorker);
        repo.setCurrentTradeID(newID);
        // Give the worker thread a brief moment to complete write
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    data::DatabaseWorker dbWorker(pmTestDbPath);
    data::TradeIDRepository readRepo(dbWorker);
    EXPECT_EQ(readRepo.getCurrentTradeID(), newID);
}

TEST_F(TradeIDRepositoryTest, SetCurrentTradeID_DoesNotUpdateToLowerValue) {
    common::TradeID initialID = 100;
    common::TradeID lowerID = 50;

    {
        data::DatabaseWorker dbWorker(pmTestDbPath);
        data::TradeIDRepository repo(dbWorker);
        repo.setCurrentTradeID(initialID);
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    {
        data::DatabaseWorker dbWorker(pmTestDbPath);
        data::TradeIDRepository repo(dbWorker);
        repo.setCurrentTradeID(lowerID);
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    data::DatabaseWorker dbWorker(pmTestDbPath);
    data::TradeIDRepository readRepo(dbWorker);
    EXPECT_EQ(readRepo.getCurrentTradeID(), initialID);
}

TEST_F(TradeIDRepositoryTest, TruncateTradeID_ResetsToZero) {
    {
        data::DatabaseWorker dbWorker(pmTestDbPath);
        data::TradeIDRepository repo(dbWorker);
        repo.setCurrentTradeID(12345);
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    {
        data::DatabaseWorker dbWorker(pmTestDbPath);
        data::TradeIDRepository repo(dbWorker);
        EXPECT_EQ(repo.getCurrentTradeID(), 12345);
        repo.truncateTradeID();
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));
    }

    data::DatabaseWorker dbWorker(pmTestDbPath);
    data::TradeIDRepository readRepo(dbWorker);
    EXPECT_EQ(readRepo.getCurrentTradeID(), 0);
}
