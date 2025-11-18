/**
 * @file FixUtils.h
 * @brief Declares utility functions for the FIX component.
 */

#pragma once

#include "common/Types.h"
#include <map>
#include <string_view>

namespace fix {

    /**
     * @brief Splits a FIX message string view into a map of tags to values.
     * @param str The string view of the FIX message.
     * @param delimiter The delimiter character (usually SOH).
     * @return A map where keys are FIX tags and values are the corresponding
     * string views.
     */
    std::map<int, std::string_view> splitToMap(std::string_view str,
                                               char delimiter);

    /**
     * @brief Converts a character representation of an order side to the
     * common::OrderSide enum.
     * @param c The character to convert (e.g., '1' for Buy).
     * @return The corresponding common::OrderSide enum value.
     * @throws std::invalid_argument if the character is not a valid order side.
     */
    common::OrderSide charToOrderSide(char c);

    /**
     * @brief Converts a character representation of an order type to the
     * common::OrderType enum.
     * @param c The character to convert (e.g., '2' for Limit).
     * @return The corresponding common::OrderType enum value.
     * @throws std::invalid_argument if the character is not a valid order type.
     */
    common::OrderType charToOrderType(char c);

} // namespace fix
