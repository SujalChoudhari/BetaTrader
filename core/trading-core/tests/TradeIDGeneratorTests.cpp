//
// Created by sujal on 25-10-2025.
//

#include "gtest/gtest.h"
#include "trading-core/TradeIDGenerator.h"
#include "logging/Logger.h"
#include "data/Constant.h"
#include "data/DatabaseWorker.h"

using namespace trading_core;

class TradeIDGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        logging::Logger::Init("trade_id_generator_test", "logs/trade_id_generator_test.log");
        dbWorker = std::make_shared<data::DatabaseWorker>(data::databasePath);
        generator = std::make_unique<TradeIDGenerator>(dbWorker);
    }

    void TearDown() override {
        generator.reset();
        logging::Logger::Shutdown();
    }

    std::shared_ptr<data::DatabaseWorker> dbWorker;
    std::unique_ptr<TradeIDGenerator> generator;
};

TEST_F(TradeIDGeneratorTest, InitialIdIsZero) {
    generator->loadState();
    EXPECT_GE(generator->getId(), 0);
}

TEST_F(TradeIDGeneratorTest, NextIdIncrementsByOne) {
    const auto id1 = generator->nextId();
    const auto id2 = generator->nextId();
    EXPECT_EQ(id2, id1 + 1);
}

TEST_F(TradeIDGeneratorTest, GetIdReturnsCurrentValue) {
    const auto current = generator->getId();
    const auto next = generator->nextId();
    EXPECT_EQ(generator->getId(), next);
    EXPECT_EQ(next, current + 1);
}

TEST_F(TradeIDGeneratorTest, ThreadSafety) {
    constexpr int num_threads = 16;
    constexpr int num_increments = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this]() {
            for (int j = 0; j < num_increments; ++j)
                generator->nextId();
        });
    }

    for (auto &t: threads)
        t.join();

    const uint64_t expected = generator->getId();
    EXPECT_GE(expected, num_threads * num_increments);
}
