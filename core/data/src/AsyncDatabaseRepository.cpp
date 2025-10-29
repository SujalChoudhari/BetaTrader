//
// Created by sujal on 27-10-2025.
//

#include "data/AsyncDatabaseRepository.h"
#include <iostream>
#include <utility>

#include "logging/Runbook.h"
#include "data/DataRunBookDefinations.h"
#include "spdlog/spdlog-inl.h"

namespace data {
    AsyncDatabaseRepository::AsyncDatabaseRepository(std::string dbPath)
        : mDbPath(std::move(dbPath)) {
        mWorker = std::thread(&AsyncDatabaseRepository::workerLoop, this);
    }

    AsyncDatabaseRepository::~AsyncDatabaseRepository() {
        {
            std::lock_guard lock(mMutex);
            mStop = true;
        }
        mCv.notify_one();
        if (mWorker.joinable())
            mWorker.join();
    }

    void AsyncDatabaseRepository::enqueue(std::function<void(SQLite::Database &)> task) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTasks.push(std::move(task));
        }
        mCv.notify_one();
    }

    void AsyncDatabaseRepository::workerLoop() {
        SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("PRAGMA journal_mode = WAL;");
        db.setBusyTimeout(1000);

        while (true) {
            std::function<void(SQLite::Database &)> task;
            {
                std::unique_lock<std::mutex> lock(mMutex);
                mCv.wait(lock, [&] { return mStop || !mTasks.empty(); });
                if (mStop && mTasks.empty()) break;
                task = std::move(mTasks.front());
                mTasks.pop();
            }

            try {
                task(db);
            } catch (const std::exception &e) {
                LOG_CRITICAL(errors::EDATA1, "Error in task (query). Details: {} ", std::string_view(e.what()));
            }
        }
    }
}
