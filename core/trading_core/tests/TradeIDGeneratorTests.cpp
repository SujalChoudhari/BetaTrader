#include <gtest/gtest.h>
#include "trading_core/TradeIDGenerator.h"
#include "data/DatabaseWorker.h"
#include "data/TradeIDRepository.h"
#include <future>
#include <chrono>

class TradeIDGeneratorTest : public ::testing::Test {
protected:
    std::unique_ptr<data::DatabaseWorker> dbWorker;
    std::unique_ptr<data::TradeIDRepository> tradeIDRepo;

    void SetUp() override {
        // Use an in-memory database for tests
        dbWorker = std::make_unique<data::DatabaseWorker>(":memory:");
        tradeIDRepo = std::make_unique<data::TradeIDRepository>(dbWorker.get());
        // Ensure the database table is created and the ID is reset before each test
        tradeIDRepo->truncateTradeID();
        dbWorker->waitUntilIdle(); // Wait for truncate to finish
    }

    void TearDown() override {
        if (dbWorker) {
            dbWorker->waitUntilIdle(); // Wait for all pending tasks to complete
            dbWorker.reset(); // Now it's safe to destroy the worker
        }
    }
};

TEST_F(TradeIDGeneratorTest, InitialStateLoading) {
    // 1. Manually set a starting ID in the database
    const common::TradeID start_id = 100;
    tradeIDRepo->setCurrentTradeID(start_id);
    dbWorker->waitUntilIdle(); // Wait for the write to complete

    // 2. Construct the generator, which triggers the asynchronous loadState()
    trading_core::TradeIDGenerator generator(dbWorker.get());

    // 3. Poll until the loaded value is reflected. This is the key to testing the race condition.
    for (int i = 0; i < 100; ++i) { // Poll for a max of 1 second
        if (generator.getId() == start_id) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 4. Assert that the state was loaded correctly
    ASSERT_EQ(generator.getId(), start_id);
}

TEST_F(TradeIDGeneratorTest, SequentialGenerationAfterLoading) {
    const common::TradeID start_id = 50;
    tradeIDRepo->setCurrentTradeID(start_id);
    dbWorker->waitUntilIdle(); // Wait for write to complete

    trading_core::TradeIDGenerator generator(dbWorker.get());

    // Poll to wait for load
    for (int i = 0; i < 100; ++i) {
        if (generator.getId() == start_id) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_EQ(generator.getId(), start_id);

    // Verify sequential generation
    EXPECT_EQ(generator.nextId(), start_id + 1);
    EXPECT_EQ(generator.nextId(), start_id + 2);
    EXPECT_EQ(generator.getId(), start_id + 2);
}

TEST_F(TradeIDGeneratorTest, StatePersistenceOnDestruction) {
    const common::TradeID final_id = 5;

    {
        // Create generator in a limited scope
        trading_core::TradeIDGenerator generator(dbWorker.get());
        // Poll to wait for initial load to complete
        for (int i = 0; i < 100; ++i) {
            if (generator.getId() == 0) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        for (common::TradeID i = 0; i < final_id; ++i) {
            generator.nextId();
        }
        ASSERT_EQ(generator.getId(), final_id);
        // Destructor is called here, triggering saveState()
    }

    // The save is async, so we must wait for it to land in the database.
    // The TearDown method will now handle this wait.

    // Verify the saved state by using the repository directly
    std::promise<common::TradeID> read_promise;
    std::future<common::TradeID> read_future = read_promise.get_future();
    tradeIDRepo->getCurrentTradeID([&](common::TradeID id) {
        read_promise.set_value(id);
    });

    // We must still wait for the read operation itself to complete
    dbWorker->waitUntilIdle();

    EXPECT_EQ(read_future.get(), final_id);
}
