/**
 * @file SequenceReset.h
 * @brief Defines the structure for a FIX Sequence Reset message.
 */

#pragma once

#include "common_fix/Types.h"

namespace fix
{

/**
 * @brief Represents a parsed FIX Sequence Reset (35=4) message.
 *
 * This struct encapsulates the fields required to either perform a gap fill
 * or forcibly reset the sequence number of a session.
 */
struct SequenceReset
{
    /** @brief The new sequence number for the session (NewSeqNo, FIX Tag 36). */
    fix::SequenceNumber newSeqNo;

    /** @brief Indicates if this reset is part of a gap fill (GapFillFlag, FIX Tag 123).
     *         'Y' for Gap Fill, 'N' for Reset. Defaults to 'N'.
     */
    char gapFillFlag;
};

} // namespace fix
