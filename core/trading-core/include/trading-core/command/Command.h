//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "CommandType.h"
#include "common/Time.h"

namespace trading_core {
    /**
     * An Internal representation of the request, converted from an FIX message into this object for faster processing
     */
    class Command {
    public:
        Command(const CommandType type, const common::Timestamp timestamp)
            : mType(type), mTimestamp(timestamp) {
        }

        CommandType getType() const { return mType; }

        common::Timestamp getTimestamp() const { return mTimestamp; }

    private:
        CommandType mType;
        common::Timestamp mTimestamp;
    };
}
