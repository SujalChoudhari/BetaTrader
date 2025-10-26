//
// Created by sujal on 25-10-2025.
//


#include "trading-core/TradeIDGenerator.h"

#include "common/Types.h"


namespace trading_core {
    common::TradeID TradeIDGenerator::currentId = 0;
    std::mutex TradeIDGenerator::mutex;


    common::TradeID TradeIDGenerator::getId() {
        return TradeIDGenerator::currentId;
    }

    common::TradeID TradeIDGenerator::nextId() {
        std::lock_guard<std::mutex> lock(mutex);
        return ++currentId;
    }

    void TradeIDGenerator::saveState() {
        // TODO: implement save and load state
    }

    void TradeIDGenerator::loadState() {
    }
}
