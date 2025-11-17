/**
 * @file BinaryToModifyOrderRequestConverter.h
 * @brief Declares the BinaryToModifyOrderRequestConverter class for converting FIX messages to ModifyOrder objects.
 */

#pragma once

#include <string>
#include <optional>
#include "fix/ModifyOrder.h"

namespace fix
{

/**
 * @brief Converts a raw FIX message string into a `ModifyOrder` object.
 *
 * This class provides a static method to parse a FIX "Order Cancel Replace Request" (MsgType=G)
 * message string and extract the relevant fields to construct a `ModifyOrder`.
 */
class BinaryToModifyOrderRequestConverter
{
public:
    /**
     * @brief Converts a raw FIX message string into a `ModifyOrder` object.
     *
     * Parses the given FIX message string. If the message is a valid "Order Cancel Replace Request"
     * and all required fields are present and correctly formatted, a `ModifyOrder`
     * object is returned. Otherwise, `std::nullopt` is returned.
     *
     * @param fixMessage The raw FIX message string to convert.
     * @return An `std::optional<ModifyOrder>` containing the parsed modify order
     *         if successful, or `std::nullopt` if parsing fails.
     */
    static std::optional<ModifyOrder> convert(const std::string& fixMessage);
};

} // namespace fix
