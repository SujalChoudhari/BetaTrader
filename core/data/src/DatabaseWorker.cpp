//
// Created by sujal on 27-10-2025.
//
#include "data/DatabaseWorker.h"
#include "logging/Runbook.h"
#include "data/DataRunBookDefinations.h"

namespace data {

    DatabaseWorker::DatabaseWorker(std::string dbPath)
        : mDbPath(std::move(dbPath)), mTasks(1024) {
        mWorker = std::jthread([this](std::stop_token st) { this->workerLoop(st); });
    }

    DatabaseWorker::~DatabaseWorker() {
        mWorker.request_stop();
        if (mWorker.joinable())
            mWorker.join();
    }

    void DatabaseWorker::enqueue(std::function<void(SQLite::Database&)> task) {
        mTasks.push(std::move(task));
    }

    size_t DatabaseWorker::getQueueSize() const {
        return mTasks.size();
    }

    void DatabaseWorker::workerLoop(std::stop_token stopToken) {
        SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("PRAGMA journal_mode = WAL;");
        db.exec("PRAGMA synchronous = NORMAL;");
        db.setBusyTimeout(5000);

        while (!stopToken.stop_requested()) {
            std::function<void(SQLite::Database&)> task;
            if (mTasks.front()) {
                task = std::move(*mTasks.front());
                mTasks.pop();
            } else {
                std::this_thread::yield();
                continue;
            }

            try {
                task(db);
            } catch (const std::exception& e) {
                LOG_CRITICAL(errors::EDATA1, "Error in async DB task: {}", std::string_view(e.what()));
            }
        }
    }

}
