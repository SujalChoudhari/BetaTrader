//
// Created by sujal on 23-10-2025.
//

#pragma once
#include <vector>

#include "Order.h"
#include "OrderBook.h"
#include "Trade.h"

namespace trading_core {
    class Matcher {
    public:
        std::vector<Trade> match(Order &incomingOrder, OrderBook &orderBook) const;

    private:
        template<typename TMap>
        void matchTable(Order &incomingOrder, std::shared_ptr<TMap> restingMap, std::vector<Trade> &trades) const;
    };
}
