#include "data/DatabaseWorker.h"
#include "data/DataRunBookDefinations.h"
#include "logging/Runbook.h"

namespace data {

    DatabaseWorker::DatabaseWorker(std::string dbPath)
        : mDbPath(std::move(dbPath)), mTasks(1024)
    {
        mWorker = std::jthread(
                [this](std::stop_token st) { this->workerLoop(st); });
    }

    // Protected constructor for mocking - does not start a thread
    DatabaseWorker::DatabaseWorker(): mTasks(1024) {}

    DatabaseWorker::~DatabaseWorker()
    {
        if (mWorker.joinable()) { mWorker.request_stop(); }
    }

    void DatabaseWorker::enqueue(std::function<void(SQLite::Database&)> task)
    {
        mTasks.push(std::move(task));
    }

    size_t DatabaseWorker::getQueueSize() const
    {
        return mTasks.size();
    }

    void DatabaseWorker::waitUntilIdle()
    {
        while (getQueueSize() > 0) { std::this_thread::yield(); }
    }

    void DatabaseWorker::workerLoop(std::stop_token stopToken)
    {
        SQLite::Database db(mDbPath,
                            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("PRAGMA journal_mode = WAL;");
        db.exec("PRAGMA synchronous = NORMAL;");
        db.setBusyTimeout(5000);

        while (!stopToken.stop_requested()) {
            std::function<void(SQLite::Database&)>* task = mTasks.front();
            if (task) {
                (*task)(db);
                mTasks.pop();
            }
            else {
                std::this_thread::yield();
            }
        }
    }

} // namespace data
