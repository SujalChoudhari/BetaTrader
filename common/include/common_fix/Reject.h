/**
 * @file Reject.h
 * @brief Defines the structure for a FIX Session-Level Reject message.
 */

#pragma once

#include "common_fix/Types.h"
#include <string>

namespace fix
{

/**
 * @brief Represents the data required to build a FIX Session-Level Reject (35=3) message.
 *
 * This struct is used to communicate session-level protocol violations.
 */
struct Reject
{
    /** @brief The sequence number of the message being rejected (RefSeqNum, FIX Tag 45). */
    fix::SequenceNumber refSeqNum;

    /** @brief A human-readable explanation of the rejection reason (Text, FIX Tag 58). */
    std::string text;

    // Other fields like RefTagID(371), SessionRejectReason(373) can be added here.
};

} // namespace fix
