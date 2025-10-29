//
// Created by sujal on 27-10-2025.
//

#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <SQLiteCpp/SQLiteCpp.h>

namespace data {

    class DatabaseWorker {
    public:
        explicit DatabaseWorker(std::string dbPath);
        ~DatabaseWorker();

        void enqueue(std::function<void(SQLite::Database&)> task);

    private:
        void workerLoop();

        std::string mDbPath;
        std::thread mWorker;
        std::mutex mMutex;
        std::condition_variable mCv;
        std::queue<std::function<void(SQLite::Database&)>> mTasks;
        bool mStop{false};
    };

    using DatabaseWorkerPtr = std::shared_ptr<DatabaseWorker>;

}