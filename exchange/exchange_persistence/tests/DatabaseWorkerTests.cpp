#include "data/DatabaseWorker.h"
#include <gtest/gtest.h>
#include <atomic>
#include <filesystem>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

TEST(DatabaseWorkerTests, AcceptsTasksFromMultipleProducerThreads) {
    const auto databasePath = std::filesystem::temp_directory_path()
                               / ("betatrader-database-worker-test-"
                                  + std::to_string(getpid()) + ".db");
    std::filesystem::remove(databasePath);

    {
        data::DatabaseWorker worker(databasePath.string());
        std::atomic<int> executed{0};
        constexpr int producerCount = 8;
        constexpr int tasksPerProducer = 100;
        std::vector<std::thread> producers;
        producers.reserve(producerCount);

        for (int producer = 0; producer < producerCount; ++producer) {
            producers.emplace_back([&worker, &executed] {
                for (int task = 0; task < tasksPerProducer; ++task) {
                    worker.enqueue([&executed](SQLite::Database&) { ++executed; });
                }
            });
        }
        for (auto& producer : producers) producer.join();

        worker.sync();
        EXPECT_EQ(executed.load(), producerCount * tasksPerProducer);
    }

    std::filesystem::remove(databasePath);
    std::filesystem::remove(databasePath.string() + "-wal");
    std::filesystem::remove(databasePath.string() + "-shm");
}