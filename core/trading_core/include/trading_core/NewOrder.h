//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Command.h"
#include "common/Order.h"

namespace trading_core {
    class NewOrder : public Command {
    public:
        explicit NewOrder(const CommandType type, const common::ClientID &clientId, const common::Timestamp timestamp,
                          common::Order order) : Command(type, clientId, timestamp),
                                                 mOrder(std::move(order)) {
        };

        [[nodiscard]] const common::Order &getOrder() const { return mOrder; }

    private:
        common::Order mOrder;
    };
}
