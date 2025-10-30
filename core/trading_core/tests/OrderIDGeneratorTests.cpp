//
// Created by sujal on 25-10-2025.
//


#include "gtest/gtest.h"
#include "trading_core/OrderIDGenerator.h"
using namespace trading_core;

TEST(OrderIDGeneratorTest, NextIdIncrementsByOne) {
    const auto id1 = OrderIDGenerator::nextId();
    const auto id2 = OrderIDGenerator::nextId();
    EXPECT_EQ(id2, id1 + 1);
}

TEST(OrderIDGeneratorTest, GetIdReturnsCurrentValue) {
    const auto current = OrderIDGenerator::getId();
    const auto next = OrderIDGenerator::nextId();
    EXPECT_EQ(OrderIDGenerator::getId(), next);
    EXPECT_EQ(next, current + 1);
}

TEST(OrderIDGeneratorTest, ThreadSafety) {
    constexpr int num_threads = 16;
    constexpr int num_increments = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([]() {
            for (int j = 0; j < num_increments; ++j)
                OrderIDGenerator::nextId();
        });
    }

    for (auto &t: threads)
        t.join();

    // Expected total increments since test start
    const uint64_t expected = OrderIDGenerator::getId();
    EXPECT_GE(expected, num_threads * num_increments);
}
