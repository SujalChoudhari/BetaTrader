/**
 * @file BinaryToCancelOrderRequestConverter.h
 * @brief Declares the BinaryToCancelOrderRequestConverter class for converting
 * FIX messages to CancelOrderRequest objects.
 */

#pragma once

#include "common_fix/CancelOrderRequest.h"
#include <optional>
#include <string>

namespace fix {

    /**
     * @brief Converts a raw FIX message string into a `CancelOrderRequest`
     * object.
     *
     * This class provides a static method to parse a FIX "Order Cancel Request"
     * (MsgType=F) message string and extract the relevant fields to construct a
     * `CancelOrderRequest`.
     */
    class BinaryToCancelOrderRequestConverter {
    public:
        /**
         * @brief Converts a raw FIX message string into a `CancelOrderRequest`
         * object.
         *
         * Parses the given FIX message string. If the message is a valid "Order
         * Cancel Request" and all required fields are present and correctly
         * formatted, a `CancelOrderRequest` object is returned. Otherwise,
         * `std::nullopt` is returned.
         *
         * @param fixMessage The raw FIX message string to convert.
         * @return An `std::optional<CancelOrderRequest>` containing the parsed
         * cancel order request if successful, or `std::nullopt` if parsing
         * fails.
         */
        static std::optional<CancelOrderRequest>
        convert(const std::string& fixMessage);
    };

} // namespace fix
