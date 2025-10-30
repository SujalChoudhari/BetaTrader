//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Command.h"
#include "common/Order.h"
#include <memory>

namespace trading_core {
    class NewOrder : public Command {
    public:
        explicit NewOrder(const common::ClientID &clientId, const common::Timestamp timestamp,
                          std::shared_ptr<common::Order> order) : Command(CommandType::NewOrder, clientId, timestamp),
                                                 mOrder(std::move(order)) {
        };

        [[nodiscard]] std::shared_ptr<common::Order> getOrder() const { return mOrder; }

    private:
        std::shared_ptr<common::Order> mOrder;
    };
}
