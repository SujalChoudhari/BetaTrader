//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "trading-core/command/Command.h"
#include "trading-core/Order.h"

namespace trading_core {
    class CancelOrder : public Command {
    public:
        explicit CancelOrder(const CommandType type, const common::Timestamp timestamp,
                          const common::OrderID mOrderId) : Command(type, timestamp),
                                                            mOrderId(mOrderId) {
        };

        [[nodiscard]] const common::OrderID &getOrder() const { return mOrderId; }

    private:
        common::OrderID mOrderId;
    };
}
