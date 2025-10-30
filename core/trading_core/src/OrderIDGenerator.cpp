//
// Created by sujal on 25-10-2025.
//

#include "trading_core/OrderIDGenerator.h"


namespace trading_core {

    std::atomic<common::OrderID> OrderIDGenerator::currentId = 0;
    std::mutex OrderIDGenerator::mutex;


    common::OrderID OrderIDGenerator::getId() {
        return OrderIDGenerator::currentId.load(std::memory_order_relaxed);
    }

    common::OrderID OrderIDGenerator::nextId() {
        return ++currentId;
    }

    void OrderIDGenerator::saveState() {
        // TODO: implement save and load state
    }

    void OrderIDGenerator::loadState() {
    }
}
