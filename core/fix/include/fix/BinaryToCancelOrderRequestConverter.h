/**
 * @file BinaryToCancelOrderRequestConverter.h
 * @brief Declares the BinaryToCancelOrderRequestConverter class for converting FIX messages to CancelOrder objects.
 */

#pragma once

#include <string>
#include <optional>
#include "fix/CancelOrder.h"

namespace fix
{

/**
 * @brief Converts a raw FIX message string into a `CancelOrder` object.
 *
 * This class provides a static method to parse a FIX "Order Cancel Request" (MsgType=F)
 * message string and extract the relevant fields to construct a `CancelOrder`.
 */
class BinaryToCancelOrderRequestConverter
{
public:
    /**
     * @brief Converts a raw FIX message string into a `CancelOrder` object.
     *
     * Parses the given FIX message string. If the message is a valid "Order Cancel Request"
     * and all required fields are present and correctly formatted, a `CancelOrder`
     * object is returned. Otherwise, `std::nullopt` is returned.
     *
     * @param fixMessage The raw FIX message string to convert.
     * @return An `std::optional<CancelOrder>` containing the parsed cancel order
     *         if successful, or `std::nullopt` if parsing fails.
     */
    static std::optional<CancelOrder> convert(const std::string& fixMessage);
};

} // namespace fix
