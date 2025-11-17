#include "data/DatabaseWorker.h"
#include "data/TradeIDRepository.h"
#include <future>
#include <gtest/gtest.h>

using namespace data;
using namespace common;

class TradeIDRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        dbWorker = std::make_unique<DatabaseWorker>(":memory:");
        tradeIDRepository = std::make_unique<TradeIDRepository>(dbWorker.get());
    }

    std::unique_ptr<DatabaseWorker> dbWorker;
    std::unique_ptr<TradeIDRepository> tradeIDRepository;
};

TEST_F(TradeIDRepositoryTest, InitDatabase)
{
    // The initDatabase is called in the constructor, so we just need to check
    // if the table exists.
    dbWorker->enqueue([](SQLite::Database& db) {
        ASSERT_TRUE(db.tableExists("trade_id_sequence"));
    });
}

TEST_F(TradeIDRepositoryTest, GetCurrentTradeID)
{
    std::promise<TradeID> promise;
    std::future<TradeID> future = promise.get_future();

    tradeIDRepository->getCurrentTradeID(
            [&](TradeID id) { promise.set_value(id); });

    ASSERT_EQ(future.get(), 0);
}

TEST_F(TradeIDRepositoryTest, SetCurrentTradeID)
{
    tradeIDRepository->setCurrentTradeID(100);

    std::promise<TradeID> promise;
    std::future<TradeID> future = promise.get_future();

    tradeIDRepository->getCurrentTradeID(
            [&](TradeID id) { promise.set_value(id); });

    ASSERT_EQ(future.get(), 100);
}

TEST_F(TradeIDRepositoryTest, TruncateTradeID)
{
    tradeIDRepository->setCurrentTradeID(100);
    tradeIDRepository->truncateTradeID();

    std::promise<TradeID> promise;
    std::future<TradeID> future = promise.get_future();

    tradeIDRepository->getCurrentTradeID(
            [&](TradeID id) { promise.set_value(id); });

    ASSERT_EQ(future.get(), 0);
}
