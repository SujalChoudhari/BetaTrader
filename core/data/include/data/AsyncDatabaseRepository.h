//
// Created by sujal on 27-10-2025.
//

#pragma once
#include "SQLiteCpp/Database.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace data {
    class AsyncDatabaseRepository {
    public:
        explicit AsyncDatabaseRepository(const std::string &dbPath);

        virtual ~AsyncDatabaseRepository();

        // Queue a database task
        void enqueue(std::function<void(SQLite::Database &)> task);

    protected:
        // For derived classes to access the database path
        std::string mDbPath;

    private:
        void workerLoop();

        std::thread mWorker;
        std::mutex mMutex;
        std::condition_variable mCv;
        std::queue<std::function<void(SQLite::Database &)> > mTasks;
        bool mStop = false;
    };
}
