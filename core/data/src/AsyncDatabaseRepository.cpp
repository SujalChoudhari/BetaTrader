//
// Created by sujal on 27-10-2025.
//

#include "data/AsyncDatabaseRepository.h"
#include <iostream>

namespace data {
    AsyncDatabaseRepository::AsyncDatabaseRepository(const std::string &dbPath)
        : mDbPath(dbPath) {
        // Start dedicated worker thread
        mWorker = std::thread(&AsyncDatabaseRepository::workerLoop, this);
    }

    AsyncDatabaseRepository::~AsyncDatabaseRepository() {
        {
            std::lock_guard<std::mutex> lock(mMutex);
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
                std::cerr << "DB worker error: " << e.what() << "\n";
            }
        }
    }
}
