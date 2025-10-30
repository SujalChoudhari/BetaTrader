//
// Created by sujal on 30-10-2025.
//

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
    inline constexpr runbook::ErrorDefinition ETRADE1{
        "ETRADE1",
        "Unknown command type detected",
        "An Unknown or un processable command type was passed with the command,"
        " view the command and/or change the type"
    };

    inline constexpr runbook::ErrorDefinition ETRADE2{
        "ETRADE2",
        "Command did not have Order inside it, expected an order for the given type of command",
        "Either change the type of command or else, pass in an order for the command"
    };

    inline constexpr runbook::ErrorDefinition ETRADE3{
        "ETRADE3",
        "Unknown command type",
        "The command type is not supported, check if the implementation of virtual functions."
    };
} // namespace errors
