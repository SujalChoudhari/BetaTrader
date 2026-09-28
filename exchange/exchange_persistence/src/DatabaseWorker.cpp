#include "data/DatabaseWorker.h"
#include "common_data/DataRunBookDefinations.h"
#include "logging/Runbook.h"
#include <chrono>
#include <future>

namespace data {

    DatabaseWorker::DatabaseWorker(std::string dbPath)
        : mDbPath(std::move(dbPath))
    {
        mWorker = std::jthread(
                [this](std::stop_token st) { this->workerLoop(st); });
    }

    // Protected constructor for mocking - does not start a thread
    DatabaseWorker::DatabaseWorker() = default;

    DatabaseWorker::~DatabaseWorker()
    {
        if (mWorker.joinable()) {
            mWorker.request_stop();
            mWorker.join();
        }
    }

    void DatabaseWorker::enqueue(std::function<void(SQLite::Database&)> task)
    {
        {
            std::lock_guard<std::mutex> lock(mTasksMutex);
            mTasks.push_back(std::move(task));
        }
        mTasksCondition.notify_one();
    }

    size_t DatabaseWorker::getQueueSize() const
    {
        std::lock_guard<std::mutex> lock(mTasksMutex);
        return mTasks.size();
    }

    void DatabaseWorker::waitUntilIdle()
    {
        while (getQueueSize() > 0) { std::this_thread::yield(); }
    }

    void DatabaseWorker::sync()
    {
        std::promise<void> promise;
        auto future = promise.get_future();
        enqueue([&promise](SQLite::Database&) {
            promise.set_value();
        });
        future.wait();
    }

    void DatabaseWorker::workerLoop(std::stop_token stopToken)
    {
        SQLite::Database db(mDbPath,
                            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("PRAGMA journal_mode = WAL;");
        db.exec("PRAGMA synchronous = NORMAL;");
        db.setBusyTimeout(5000);

        while (!stopToken.stop_requested()) {
            std::function<void(SQLite::Database&)> task;
            {
                std::unique_lock<std::mutex> lock(mTasksMutex);
                mTasksCondition.wait_for(lock, std::chrono::milliseconds(10), [&] {
                    return !mTasks.empty() || stopToken.stop_requested();
                });
                if (mTasks.empty()) continue;
                task = std::move(mTasks.front());
                mTasks.pop_front();
            }
            task(db);
        }
    }

} // namespace data
