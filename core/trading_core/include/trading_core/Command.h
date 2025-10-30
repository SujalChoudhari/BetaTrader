//
// Created by sujal on 21-10-2025.
//

#pragma once
#include <utility>

#include "common/Order.h"
#include "trading_core/CommandType.h"
#include "common/Time.h"
#include "common/Types.h"

namespace trading_core {
    /**
     * An Internal representation of the request, converted from an FIX message into this object for faster processing
     */
    class Command {
    public:
        Command(const CommandType type, common::ClientID clientId, const common::Timestamp timestamp)
            : mType(type), mClientId(std::move(clientId)), mTimestamp(timestamp) {
        }

        virtual ~Command() = default;


        [[nodiscard]] CommandType getType() const { return mType; }
        [[nodiscard]] common::ClientID getClientId() const { return mClientId; }
        [[nodiscard]] common::Timestamp getTimestamp() const { return mTimestamp; }

    private:
        CommandType mType;
        common::ClientID mClientId;
        common::Timestamp mTimestamp;
    };
}
