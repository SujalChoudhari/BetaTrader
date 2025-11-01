//
// Created by sujal on 27-10-2025.
//

#pragma once
#include <string>
#include <thread>
#include <functional>
#include <SQLiteCpp/SQLiteCpp.h>
#include "rigtorp/SPSCQueue.h"

namespace data {

    class DatabaseWorker {
    public:
        explicit DatabaseWorker(std::string dbPath);
        ~DatabaseWorker();

        void enqueue(std::function<void(SQLite::Database&)> task);
        size_t getQueueSize() const;

    private:
        void workerLoop(std::stop_token stopToken);

        std::string mDbPath;
        std::jthread mWorker;
        rigtorp::SPSCQueue<std::function<void(SQLite::Database&)>> mTasks;
    };

}