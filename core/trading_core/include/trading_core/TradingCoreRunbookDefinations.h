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

    inline constexpr runbook::ErrorDefinition ETRADE4{
        "ETRADE4",
        "Incoming Order is null",
        "Matcher received a null pointer for incoming order."
    };

    inline constexpr runbook::ErrorDefinition ETRADE5{
        "ETRADE5",
        "Order not found",
        "Order not found in the order book."
    };

    inline constexpr runbook::ErrorDefinition ETRADE6{
        "ETRADE6",
        "Order not found",
        "Order not found in the order manager."
    };

    inline constexpr runbook::ErrorDefinition ETRADE7{
        "ETRADE7",
        "Invalid NewOrder cast",
        "Failed to cast Command to NewOrder."
    };

    inline constexpr runbook::ErrorDefinition ETRADE8{
        "ETRADE8",
        "Invalid ModifyOrder cast",
        "Failed to cast Command to ModifyOrder."
    };

    inline constexpr runbook::ErrorDefinition ETRADE9{
        "ETRADE9",
        "Invalid CancelOrder cast",
        "Failed to cast Command to CancelOrder."
    };

    inline constexpr runbook::ErrorDefinition ETRADE10{
        "ETRADE10",
        "Risk check failed",
        "The order has been rejected by the risk manager."
    };
} // namespace errors
