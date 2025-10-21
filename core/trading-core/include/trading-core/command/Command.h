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
        Command(CommandType type, common::Timestamp timestamp);

        CommandType getType() const;

        common::Timestamp getTimestamp() const;

    private:
        CommandType mType;
        common::Timestamp mTimestamp;
    };
}
