//
// Created by sujal on 27-10-2025.
//
#include "data/DatabaseWorker.h"
#include "logging/Runbook.h"
#include "data/DataRunBookDefinations.h"

namespace data {

    DatabaseWorker::DatabaseWorker(std::string dbPath)
        : mDbPath(std::move(dbPath)) {
        mWorker = std::thread(&DatabaseWorker::workerLoop, this);
    }

    DatabaseWorker::~DatabaseWorker() {
        {
            std::lock_guard lock(mMutex);
            mStop = true;
        }
        mCv.notify_one();
        if (mWorker.joinable())
            mWorker.join();
    }

    void DatabaseWorker::enqueue(std::function<void(SQLite::Database&)> task) {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTasks.push(std::move(task));
        }
        mCv.notify_one();
    }

    void DatabaseWorker::workerLoop() {
        SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("PRAGMA journal_mode = WAL;");
        db.exec("PRAGMA synchronous = NORMAL;");
        db.setBusyTimeout(5000);

        while (true) {
            std::function<void(SQLite::Database&)> task;
            {
                std::unique_lock<std::mutex> lock(mMutex);
                mCv.wait(lock, [&] { return mStop || !mTasks.empty(); });
                if (mStop && mTasks.empty()) break;
                task = std::move(mTasks.front());
                mTasks.pop();
            }

            try {
                task(db);
            } catch (const std::exception& e) {
                LOG_CRITICAL(errors::EDATA1, "Error in async DB task: {}", std::string_view(e.what()));
            }
        }
    }

}
