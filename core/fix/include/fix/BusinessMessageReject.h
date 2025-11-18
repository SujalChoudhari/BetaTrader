/**
 * @file BusinessMessageReject.h
 * @brief Defines the structure for a FIX Business Message Reject message.
 */

#pragma once

#include "common_fix/Types.h"
#include <string>

namespace fix
{

/**
 * @brief Represents the data required to build a FIX Business Message Reject (35=j) message.
 *
 * This struct is used to reject an application-level message that is syntactically correct
 * but violates a business rule.
 */
struct BusinessMessageReject
{
    /** @brief The sequence number of the application message being rejected (RefSeqNum, FIX Tag 45). */
    fix::SequenceNumber refSeqNum;

    /** @brief The MsgType of the message being rejected (RefMsgType, FIX Tag 372). */
    char refMsgType;

    /** @brief The categorical reason for the rejection (BusinessRejectReason, FIX Tag 379). */
    int businessRejectReason;

    /** @brief A human-readable explanation of the rejection reason (Text, FIX Tag 58). */
    std::string text;
};

} // namespace fix
