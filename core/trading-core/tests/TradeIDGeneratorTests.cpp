//
// Created by sujal on 25-10-2025.
//


#include "gtest/gtest.h"
#include "trading-core/TradeIDGenerator.h"
using namespace trading_core;

TEST(TradeIDGeneratorTest, InitialIdIsZero) {
    TradeIDGenerator::LoadState();
    EXPECT_NE(TradeIDGenerator::GetId(), 0);
}

TEST(TradeIDGeneratorTest, NextIdIncrementsByOne) {
    const auto id1 = TradeIDGenerator::NextId();
    const auto id2 = TradeIDGenerator::NextId();
    EXPECT_EQ(id2, id1 + 1);
}

TEST(TradeIDGeneratorTest, GetIdReturnsCurrentValue) {
    const auto current = TradeIDGenerator::GetId();
    const auto next = TradeIDGenerator::NextId();
    EXPECT_EQ(TradeIDGenerator::GetId(), next);
    EXPECT_EQ(next, current + 1);
}

TEST(TradeIDGeneratorTest, ThreadSafety) {
    constexpr int num_threads = 16;
    constexpr int num_increments = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([]() {
            for (int j = 0; j < num_increments; ++j)
                TradeIDGenerator::NextId();
        });
    }

    for (auto &t: threads)
        t.join();

    const uint64_t expected = TradeIDGenerator::GetId();
    EXPECT_GE(expected, num_threads * num_increments);
}
