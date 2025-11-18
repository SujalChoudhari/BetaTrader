/**
 * @file LogonRequest.h
 * @brief Defines the structure for a FIX Logon message.
 */

#pragma once

#include "common_fix/Types.h"
#include <chrono>

namespace fix
{

/**
 * @brief Represents a parsed FIX Logon (35=A) message.
 *
 * This struct encapsulates the necessary fields from a raw FIX message
 * for a client to log into a session.
 */
struct LogonRequest
{
    /** @brief Heartbeat Interval in seconds (HeartBtInt, FIX Tag 108). */
    uint32_t heartBtInt;

    /** @brief Sender's component ID (SenderCompID, FIX Tag 49). */
    fix::CompID senderCompID;

    /** @brief Target's component ID (TargetCompID, FIX Tag 56). */
    fix::CompID targetCompID;

    // Other fields like EncryptMethod(98), RawData(96), etc., can be added here if needed.
};

} // namespace fix
