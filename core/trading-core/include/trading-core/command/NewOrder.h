//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "trading-core/command/Command.h"
#include "trading-core/Order.h"

namespace trading_core {
    class NewOrder : public Command {
    public:
        explicit NewOrder(const CommandType type, const common::Timestamp timestamp, Order order) : Command(type, timestamp),
            mOrder(std::move(order)) {
        };

        const Order &getOrder() const { return mOrder; }

    private:
        Order mOrder;
    };
}
