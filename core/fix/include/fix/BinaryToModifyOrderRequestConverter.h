/**
 * @file BinaryToModifyOrderRequestConverter.h
 * @brief Declares the BinaryToModifyOrderRequestConverter class for converting
 * FIX messages to ModifyOrderRequest objects.
 */

#pragma once

#include "common_fix/ModifyOrderRequest.h"
#include <optional>
#include <string>

namespace fix {

    /**
     * @brief Converts a raw FIX message string into a `ModifyOrderRequest`
     * object.
     *
     * This class provides a static method to parse a FIX "Order Cancel Replace
     * Request" (MsgType=G) message string and extract the relevant fields to
     * construct a `ModifyOrderRequest`.
     */
    class BinaryToModifyOrderRequestConverter {
    public:
        /**
         * @brief Converts a raw FIX message string into a `ModifyOrderRequest`
         * object.
         *
         * Parses the given FIX message string. If the message is a valid "Order
         * Cancel Replace Request" and all required fields are present and
         * correctly formatted, a `ModifyOrderRequest` object is returned.
         * Otherwise, `std::nullopt` is returned.
         *
         * @param fixMessage The raw FIX message string to convert.
         * @return An `std::optional<ModifyOrderRequest>` containing the parsed
         * modify order request if successful, or `std::nullopt` if parsing
         * fails.
         */
        static std::optional<ModifyOrderRequest>
        convert(const std::string& fixMessage);
    };

} // namespace fix
