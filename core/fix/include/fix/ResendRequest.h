/**
 * @file ResendRequest.h
 * @brief Defines the structure for a FIX Resend Request message.
 */

#pragma once

#include "common_fix/Types.h"

namespace fix
{

/**
 * @brief Represents a parsed FIX Resend Request (35=2) message.
 *
 * This struct encapsulates the fields required to request the retransmission
 * of a range of messages.
 */
struct ResendRequest
{
    /** @brief The sequence number of the first message in the range to be resent (BeginSeqNo, FIX Tag 7). */
    fix::SequenceNumber beginSeqNo;

    /** @brief The sequence number of the last message in the range to be resent (EndSeqNo, FIX Tag 16).
     *         A value of 0 means "to the end of the session".
     */
    fix::SequenceNumber endSeqNo;
};

} // namespace fix
