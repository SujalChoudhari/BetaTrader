/**
 * @file DatabaseWorker.h
 * @brief Asynchronous database worker abstraction.
 *
 * This worker accepts lambda tasks that operate on a SQLite::Database instance
 * and executes them on a background thread. It provides a queue interface to
 * submit database tasks without blocking the caller.
 */

#pragma once
#include "rigtorp/SPSCQueue.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <functional>
#include <string>
#include <thread>

namespace data {
    /**
     * @class DatabaseWorker
     * @brief Executes database tasks asynchronously.
     */
    class DatabaseWorker {
    public:
        explicit DatabaseWorker(std::string dbPath);
        virtual ~DatabaseWorker();

        virtual void enqueue(std::function<void(SQLite::Database&)> task);
        size_t getQueueSize() const;
        void waitUntilIdle();

    protected:
        // Protected constructor for mocking
        DatabaseWorker();

    private:
        void workerLoop(std::stop_token stopToken);

        std::string mDbPath;
        std::jthread mWorker;
        rigtorp::SPSCQueue<std::function<void(SQLite::Database&)>> mTasks;
    };
} // namespace data