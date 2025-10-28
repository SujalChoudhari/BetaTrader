#pragma once

#include "logging/Runbook.h"
#include <string_view>

/**
 * @brief This file defines all runbook error codes as objects.
 *
 * Include this header in any .cpp file that needs to log a runbook error.
 * The use of 'inline constexpr' (C++17) ensures that these
 * objects are defined once, at compile time, and are available
 * to be passed directly to the logger.
 *
 * This new structure is fully type-safe and avoids all
 * linker and runtime "Unknown Error" issues.
 */
namespace errors {

    // --- Define all your error codes as ErrorDefinition objects ---
    // This is now the one and only place you need to define an error.

    inline constexpr runbook::ErrorDefinition E1001 {
        "E-1001",
        "Failed to connect to the primary database.",
        "1. Check that the DB_HOST and DB_PORT environment variables are set correctly.\n"
        "    2. Verify the database server is running and accessible (check firewalls).\n"
        "    3. Check database credentials in the configuration."
    };

    inline constexpr runbook::ErrorDefinition W2005 {
        "W-2005",
        "Market data feed is stale.",
        "1. Check connection to the market data provider.\n"
        "    2. See if the market is currently closed or in auction."
    };

    inline constexpr runbook::ErrorDefinition C9999 {
        "C-9999",
        "A critical, undefined failure occurred.",
        "1. This is a catch-all critical error. Check logs immediately preceding this one.\n"
        "    2. Restart the service and monitor closely."
    };

    // ... add all other error objects here

} // namespace errors

