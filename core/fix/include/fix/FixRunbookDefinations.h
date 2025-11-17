/**
 * @file FixRunbookDefinations.h
 * @brief Runbook error definitions for the FIX server module.
 *
 * Declares `runbook::ErrorDefinition` objects for FIX server-specific failures
 * so callers can log structured runbook messages with guidance for remediation.
 */

#pragma once

#include "logging/Runbook.h"

namespace errors {
    /**
     * @brief Error definition for when the FIX Server fails to start.
     */
    inline constexpr runbook::ErrorDefinition EFIX1{
            "EFIX1", "FIX Server failed to start.",
            "Check if the port is already in use or if there are issues with network permissions."};

    /**
     * @brief Error definition for when an incoming FIX message cannot be processed.
     */
    inline constexpr runbook::ErrorDefinition EFIX2{
            "EFIX2", "Failed to process incoming FIX message.",
            "The message may be malformed or the checksum validation failed. Check the raw message data."};
    
    /**
     * @brief Error definition for when a FIX session disconnects with an error.
     */
    inline constexpr runbook::ErrorDefinition EFIX3{
            "EFIX3", "Session disconnected with an error.",
            "This could be due to a network issue or an ungraceful disconnect from the client."};

} // namespace errors
