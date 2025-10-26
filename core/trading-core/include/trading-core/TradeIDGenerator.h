//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <mutex>

#include "common/Types.h"
#include "data/TradeIDRepository.h"

namespace trading_core {
    class TradeIDGenerator {
        static common::TradeID currentId;
        static std::mutex mutex;
        static data::TradeIDRepository repository;

    public:
        static common::TradeID getId();

        static common::TradeID nextId();

        static void saveState();

        static void loadState();
    };

}
