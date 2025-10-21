//
// Created by sujal on 21-10-2025.
//

#include "trading-core/command/Command.h"

namespace trading_core {
    Command::Command(const CommandType type, const common::Timestamp timestamp) : mType(type), mTimestamp(timestamp) {
    }

    CommandType Command::getType() const {
        return mType;
    }

    common::Timestamp Command::getTimestamp() const {
        return mTimestamp;
    }
}
