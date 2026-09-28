#include "data/DatabaseWorker.h"
#include "data/MarketHistoryRepository.h"
#include "ohlc/CandleAggregator.h"
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

class CandleAggregatorTest : public ::testing::Test {
protected:
    std::filesystem::path databasePath;
    std::unique_ptr<data::DatabaseWorker> databaseWorker;
    std::unique_ptr<data::MarketHistoryRepository> repository;

    void SetUp() override {
        databasePath = std::filesystem::temp_directory_path()
                       / ("betatrader-candle-test-" + std::to_string(getpid()) + ".db");
        std::filesystem::remove(databasePath);
        std::filesystem::remove(databasePath.string() + "-wal");
        std::filesystem::remove(databasePath.string() + "-shm");
        databaseWorker = std::make_unique<data::DatabaseWorker>(databasePath.string());
        repository = std::make_unique<data::MarketHistoryRepository>(databaseWorker.get());
    }

    void TearDown() override {
        repository.reset();
        databaseWorker.reset();
        std::filesystem::remove(databasePath);
        std::filesystem::remove(databasePath.string() + "-wal");
        std::filesystem::remove(databasePath.string() + "-shm");
    }
};

} // namespace

TEST_F(CandleAggregatorTest, PublishesActiveCandleImmediatelyAndOnUpdates) {
    ohlc::CandleAggregator aggregator(*repository);
    std::vector<data::Candle> published;
    aggregator.setCandleCallback([&published](int interval, const data::Candle& candle) {
        if (interval == 1) published.push_back(candle);
    });

    constexpr int64_t firstBucketSeconds = 1'700'000'000;
    const auto firstTimestamp = firstBucketSeconds * 1'000'000'000LL;
    aggregator.onTrade("EURUSD", 1.1000, 10, firstTimestamp);

    ASSERT_EQ(published.size(), 1U);
    EXPECT_DOUBLE_EQ(published.back().open, 1.1000);
    EXPECT_DOUBLE_EQ(published.back().close, 1.1000);
    EXPECT_EQ(published.back().volume, 10U);

    aggregator.onTrade("EURUSD", 1.1010, 5, firstTimestamp + 30'000'000'000LL);
    ASSERT_EQ(published.size(), 2U);
    EXPECT_DOUBLE_EQ(published.back().high, 1.1010);
    EXPECT_DOUBLE_EQ(published.back().close, 1.1010);
    EXPECT_EQ(published.back().volume, 15U);
}
