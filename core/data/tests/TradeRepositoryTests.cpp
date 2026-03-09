#include "common/Trade.h"
#include "data/DatabaseWorker.h"
#include "data/TradeRepository.h"
#include <chrono>
#include <gtest/gtest.h>

using namespace data;
using namespace common;

class TradeRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dbWorker = std::make_unique<DatabaseWorker>(":memory:");
        tradeRepository = std::make_unique<TradeRepository>(dbWorker.get());
    }

    std::unique_ptr<DatabaseWorker> dbWorker;
    std::unique_ptr<TradeRepository> tradeRepository;
};

TEST_F(TradeRepositoryTest, InitDatabase)
{
    // The initDatabase is called in the constructor, so we just need to check
    // if the table exists.
    dbWorker->enqueue([](SQLite::Database& db) {
        ASSERT_TRUE(db.tableExists("trades"));
    });
    dbWorker->sync();
}

TEST_F(TradeRepositoryTest, AddTrade)
{
    Trade trade(1, Instrument::EURUSD, 1, 2, 100, 1.1000,
                std::chrono::system_clock::now());
    tradeRepository->addTrade(trade);

    dbWorker->enqueue([&](SQLite::Database& db) {
        SQLite::Statement query(db, "SELECT * FROM trades WHERE trade_id = ?");
        query.bind(1, 1);
        ASSERT_TRUE(query.executeStep());
        ASSERT_EQ(query.getColumn(0).getInt64(), 1);
    });
    dbWorker->sync();
}
