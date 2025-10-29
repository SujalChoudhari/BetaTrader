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


    common::TradeID TradeIDGenerator::GetId() {
        if (currentId == 0)
            LoadState();
        return currentId;
    }

    common::TradeID TradeIDGenerator::NextId() {
        std::lock_guard lock(mutex);
        SaveState();
        return ++currentId;
    }

    void TradeIDGenerator::SaveState() {
        repository.setCurrentTradeID(currentId);
    }

    void TradeIDGenerator::LoadState() {
        currentId = repository.getCurrentTradeID();
    }
}
