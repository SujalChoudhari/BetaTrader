//
// Created by sujal on 25-10-2025.
//

#pragma once
#include <mutex>

#include "common/Types.h"
#include "data/TradeIDRepository.h"

namespace trading_core {
    class TradeIDGenerator {
    public:
        static common::TradeID GetId();

        static common::TradeID NextId();

        static void SaveState();

        static void LoadState();

    private:
        static common::TradeID currentId;
        static std::mutex mutex;
        static data::TradeIDRepository repository;
    };
}
