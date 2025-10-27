//
// Created by sujal on 23-10-2025.
//

#pragma once
#include <vector>

#include "OrderBook.h"
#include "common/Trade.h"

namespace trading_core {
    class Matcher {
    public:
        std::vector<common::Trade> match(common::Order &incomingOrder, OrderBook &orderBook) const;

    private:
        template<typename TMap>
        void matchTable(common::Order &incomingOrder, std::shared_ptr<TMap> restingMap, std::vector<common::Trade> &trades) const;
    };
}
