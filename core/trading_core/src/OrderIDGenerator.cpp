//
// Created by sujal on 25-10-2025.
//

#include "trading_core/OrderIDGenerator.h"


namespace trading_core {

    common::OrderID OrderIDGenerator::currentId = 0;
    std::mutex OrderIDGenerator::mutex;


    common::OrderID OrderIDGenerator::getId() {
        return OrderIDGenerator::currentId;
    }

    common::OrderID OrderIDGenerator::nextId() {
        std::lock_guard<std::mutex> lock(mutex);
        return ++currentId;
    }

    void OrderIDGenerator::saveState() {
        // TODO: implement save and load state
    }

    void OrderIDGenerator::loadState() {
    }
}
