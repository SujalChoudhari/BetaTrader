//
// Created by sujal on 26-10-2025.
//

#include "data/TradeIDRepository.h"
#include "gtest/gtest.h"

class TradeIDRepositoryTest : public testing::Test {
protected:
    void SetUp() override {
        repo = std::make_unique<data::TradeIDRepository>(":memory:");
        repo->truncateTradeID();
    }

    void TearDown() override {
        repo.reset();
    }

    std::unique_ptr<data::TradeIDRepository> repo;
};

TEST_F(TradeIDRepositoryTest, InitDatabaseDoesNotThrow) {
    EXPECT_NO_THROW(repo->initDatabase());
}

TEST_F(TradeIDRepositoryTest, GetCurrentTradeIDReturnsZeroAfterTruncate) {
    const auto tradeID = repo->getCurrentTradeID();
    EXPECT_EQ(tradeID, 0u);
}

TEST_F(TradeIDRepositoryTest, SetAndGetTradeID) {
    constexpr common::TradeID testID = 12345;
    repo->setCurrentTradeID(testID);

    const auto readID = repo->getCurrentTradeID();
    EXPECT_EQ(readID, testID);
}

TEST_F(TradeIDRepositoryTest, OverwriteTradeID) {
    repo->setCurrentTradeID(100);
    const auto readID1 = repo->getCurrentTradeID();
    EXPECT_EQ(readID1, 100u);

    repo->setCurrentTradeID(200);
    const auto readID2 = repo->getCurrentTradeID();
    EXPECT_EQ(readID2, 200u);
}
