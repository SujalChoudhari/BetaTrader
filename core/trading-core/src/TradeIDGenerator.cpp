//
// Created by sujal on 25-10-2025.
//


#include "trading-core/TradeIDGenerator.h"
#include "common/Types.h"
#include "data/Constant.h"


namespace trading_core {
    common::TradeID TradeIDGenerator::currentId = 0;
    std::mutex TradeIDGenerator::mutex;
    data::TradeIDRepository TradeIDGenerator::repository = data::TradeIDRepository(data::databasePath);


    common::TradeID TradeIDGenerator::getId() {
        if (currentId == 0)
            loadState();
        return TradeIDGenerator::currentId;
    }

    common::TradeID TradeIDGenerator::nextId() {
        std::lock_guard<std::mutex> lock(mutex);
        saveState();
        return ++currentId;
    }

    void TradeIDGenerator::saveState() {
        repository.setCurrentTradeID(currentId);
    }

    void TradeIDGenerator::loadState() {
        currentId = repository.getCurrentTradeID();
    }
}
