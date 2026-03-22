/**
 * @file BinaryToOrderRequestConverter.h
 * @brief Declares the BinaryToOrderRequestConverter class for converting FIX messages to OrderRequest objects.
 */

#pragma once

#include "common_fix/OrderRequest.h"
#include <optional>
#include <string>
#include <vector>

namespace fix
{

/**
 * @brief Converts a raw FIX message string into an `OrderRequest` object.
 *
 * This class provides a static method to parse a FIX "New Order - Single" (MsgType=D)
 * message string and extract the relevant fields to construct an `OrderRequest`.
 */
class BinaryToOrderRequestConverter
{
public:
    /**
     * @brief Converts a raw FIX message string into an `OrderRequest` object.
     *
     * Parses the given FIX message string. If the message is a valid "New Order - Single"
     * and all required fields are present and correctly formatted, an `OrderRequest`
     * object is returned. Otherwise, `std::nullopt` is returned.
     *
     * @param fixMessage The raw FIX message string to convert.
     * @return An `std::optional<OrderRequest>` containing the parsed order request
     *         if successful, or `std::nullopt` if parsing fails.
     */
    static std::optional<OrderRequest> convert(const std::string& fixMessage);
};

} // namespace fix
