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
     * @class Command
     * @brief An internal representation of a request, converted from an FIX message into this object for faster processing.
     *
     * This is the base class for all commands in the trading system. It provides a common interface for
     * handling different types of requests, such as new orders, modifications, and cancellations.
     */
    class Command {
    public:
        /**
         * @brief Constructs a new Command object.
         * @param type The type of the command.
         * @param clientId The identifier of the client sending the command.
         * @param timestamp The timestamp of when the command was created.
         */
        Command(const CommandType type, common::ClientID clientId, const common::Timestamp timestamp)
            : mType(type), mClientId(std::move(clientId)), mTimestamp(timestamp) {
        }

        /**
         * @brief Virtual destructor for the Command class.
         */
        virtual ~Command() = default;

        /** @brief Gets the type of the command. */
        [[nodiscard]] CommandType getType() const { return mType; }
        /** @brief Gets the client's identifier. */
        [[nodiscard]] common::ClientID getClientId() const { return mClientId; }
        /** @brief Gets the command's timestamp. */
        [[nodiscard]] common::Timestamp getTimestamp() const { return mTimestamp; }

    private:
        CommandType mType;              ///< The type of the command.
        common::ClientID mClientId;     ///< The identifier of the client sending the command.
        common::Timestamp mTimestamp;   ///< The timestamp of when the command was created.
    };
}
