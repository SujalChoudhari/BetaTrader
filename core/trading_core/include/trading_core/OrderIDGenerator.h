//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <mutex>
#include <atomic>

#include "common/Types.h"

namespace trading_core {
    class OrderIDGenerator {
        static std::atomic<common::OrderID> currentId;
        static std::mutex mutex;

    public:
        static common::OrderID getId();

        static common::OrderID nextId();

        static void saveState();

        static void loadState();
    };

}
