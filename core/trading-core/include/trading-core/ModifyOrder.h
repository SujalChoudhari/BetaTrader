//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Command.h"
#include "trading-core/Order.h"

namespace trading_core {
    class ModifyOrder : public Command {
    public:
        explicit ModifyOrder(const CommandType type, const common::Timestamp timestamp,
                             const common::OrderID mOrderId, const common::Price newPrice,
                             const common::Quantity newQuantity) : Command(type, timestamp),
                                                                   mOrderId(mOrderId), mNewPrice(newPrice),
                                                                   mNewQuantity(newQuantity) {
        };

        [[nodiscard]] const common::OrderID &getOrder() const { return mOrderId; }
        [[nodiscard]] const common::Price &getNewPrice() const { return mNewPrice; }
        [[nodiscard]] const common::Quantity &getNewQuantity() const { return mNewQuantity; }

    private:
        common::OrderID mOrderId;
        common::Price mNewPrice;
        common::Quantity mNewQuantity;
    };
}
