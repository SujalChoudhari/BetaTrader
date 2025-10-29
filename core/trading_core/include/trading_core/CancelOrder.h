//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "trading_core/CommandType.h"
#include "trading_core/Command.h"
#include "common/Order.h"

namespace trading_core {
    class CancelOrder : public Command {
    public:
        explicit CancelOrder(const CommandType type, const common::ClientID &clientId, const common::Timestamp timestamp,
                             const common::OrderID mOrderId) : Command(type, clientId, timestamp),
                                                               mOrderId(mOrderId) {
        };

        [[nodiscard]] const common::OrderID &getOrderId() const { return mOrderId; }

    private:
        common::OrderID mOrderId;
    };
}
