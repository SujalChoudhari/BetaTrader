//
// Created by sujal on 26-10-2025.
//

#include "data/TradeIDRepository.h"
#include "gtest/gtest.h"
#include <thread>
#include <chrono>
#include <vector>
#include <cstdio> // For std::remove
#include <set>    // For tracking written IDs
#include <string> // For dbPath

class TradeIDRepositoryTest : public testing::Test {
protected:
    // Use a file-based DB for concurrency testing, as :memory: dbs
    // are private to each connection and cannot be shared between threads.
    const std::string dbPath = "test_trade_id_repo.sqlite";

    void SetUp() override {
        // Use a pristine file-based SQLite DB for each test
        repo = std::make_unique<data::TradeIDRepository>(dbPath);

        // Constructor already initializes the table.
        // We truncate to ensure a known empty state.
        repo->truncateTradeID();
    }

    void TearDown() override {
        repo.reset();
        // Clean up the database file
        std::remove(dbPath.c_str());
    }

    std::unique_ptr<data::TradeIDRepository> repo;
};

// Test that database initializes without crashing
TEST_F(TradeIDRepositoryTest, InitDatabaseDoesNotThrow) {
    // initDatabase() is called in the constructor, but we can call it again.
    EXPECT_NO_THROW(repo->initDatabase());
}

// Test that initial trade ID is 0 after truncation
TEST_F(TradeIDRepositoryTest, GetCurrentTradeIDReturnsZeroAfterTruncate) {
    auto tradeID = repo->getCurrentTradeID();
    // After truncation, no row exists, so getCurrentTradeID should return 0.
    EXPECT_EQ(tradeID, 0u);
}

// Test writing and reading back a trade ID
TEST_F(TradeIDRepositoryTest, SetAndGetTradeID) {
    common::TradeID testID = 12345;
    repo->setCurrentTradeID(testID);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(200));

    auto readID = repo->getCurrentTradeID();
    EXPECT_EQ(readID, testID);
}

// Test that multiple writes overwrite correctly
TEST_F(TradeIDRepositoryTest, OverwriteTradeID) {
    // Test that a larger value overwrites a smaller one
    repo->setCurrentTradeID(100);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(200));

    auto readID1 = repo->getCurrentTradeID();
    EXPECT_EQ(readID1, 100u);

    repo->setCurrentTradeID(200);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(200));

    auto readID2 = repo->getCurrentTradeID();
    EXPECT_EQ(readID2, 200u);
}

// Test that "write-only-if-greater" logic works
TEST_F(TradeIDRepositoryTest, SetLowerTradeIDIsIgnored) {
    // Set an initial high value
    repo->setCurrentTradeID(500);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(200));

    auto readID1 = repo->getCurrentTradeID();
    EXPECT_EQ(readID1, 500u);

    // Attempt to set a lower value
    repo->setCurrentTradeID(300);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(200));

    auto readID2 = repo->getCurrentTradeID();

    // The value should remain the higher one
    EXPECT_EQ(readID2, 500u);
}

// Test for concurrent writes from different connections (threads)
TEST_F(TradeIDRepositoryTest, ConcurrentWrites) {
    const int numThreads = 10;
    std::vector<std::thread> threads;
    std::set<common::TradeID> writtenIDs;

    // Reset the main repo connection *before* spawning threads
    // to prevent "database is locked" errors. This closes the
    // connection held by the test fixture.
    repo.reset();

    common::TradeID maxID = 0;
    for (int i = 0; i < numThreads; ++i) {
        common::TradeID testID = 1000 + i;
        writtenIDs.insert(testID);
        if (testID > maxID) {
            maxID = testID;
        }

        threads.emplace_back([this, testID]() {
            // Each thread uses its own repository instance (connection)
            // to the same database file. SQLite handles serializing
            // the writes.
            try {
                data::TradeIDRepository threadRepo(dbPath);
                threadRepo.setCurrentTradeID(testID);
            } catch (const std::exception &e) {
                // Fail test if any thread throws
                FAIL() << "Thread exception: " << e.what();
            }
        });
    }

    for (auto &t: threads) {
        t.join();
    }

    // After all threads are done, create a new repo instance
    // to check the final value.
    data::TradeIDRepository readRepo(dbPath);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(600));

    auto finalID = readRepo.getCurrentTradeID();

    // With the new MAX() logic, the final ID is now deterministic:
    // it must be the highest ID that was written.
    EXPECT_EQ(finalID, maxID);

    // The old check is still true, but the EQ check is stronger.
    EXPECT_TRUE(writtenIDs.count(finalID))
        << "Final ID " << finalID << " was not one of the written IDs.";
}

// Test for read/write race conditions (increment)
// This tests an unsafe read-modify-write cycle.
TEST_F(TradeIDRepositoryTest, ConcurrentIncrementRaceCondition) {
    // Start with a known value
    repo->setCurrentTradeID(0);

    // Reset the main repo connection *before* spawning threads
    // to prevent "database is locked" errors.
    repo.reset();

    const int numThreads = 5;
    const int incrementsPerThread = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, incrementsPerThread]() {
            try {
                data::TradeIDRepository threadRepo(dbPath);
                for (int j = 0; j < incrementsPerThread; ++j) {
                    // This is an unsafe read-modify-write cycle
                    auto currentID = threadRepo.getCurrentTradeID();
                    threadRepo.setCurrentTradeID(currentID + 1);
                }
            } catch (const std::exception &e) {
                FAIL() << "Thread exception: " << e.what();
            }
        });
    }

    for (auto &t: threads) {
        t.join();
    }

    // Create a new repo instance to read the final value
    data::TradeIDRepository readRepo(dbPath);
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(500));
    auto finalID = readRepo.getCurrentTradeID();
    const common::TradeID expectedID = numThreads * incrementsPerThread;

    // We EXPECT the final ID to NOT be the "correct" value,
    // which demonstrates the race condition.
    EXPECT_NE(finalID, expectedID)
        << "Race condition test failed: Final ID was " << finalID
        << ", which is the expected value. This might happen "
        << "by chance or if increments are too low.";

    // The final value should be greater than 0 but less than the expected total.
    EXPECT_GT(finalID, 0u);
    EXPECT_LT(finalID, expectedID)
        << "Final ID " << finalID << " was not less than expected " << expectedID;
}
