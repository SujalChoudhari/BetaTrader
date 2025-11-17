#pragma once

#include "fix/OrderRequest.h"
#include <vector>

namespace fix {

    /**
     * @class BinaryToOrderRequestConverter
     * @brief A static utility class for deserializing a FIX message into an `OrderRequest`.
     *
     * This class handles the conversion of a raw binary FIX message (specifically
     * a New Order - Single, 35=D) into a structured `OrderRequest` object.
     */
    class BinaryToOrderRequestConverter {
    public:
        /**
         * @brief Converts a raw FIX message into an `OrderRequest`.
         * @param binaryData A vector of characters containing the raw FIX message.
         * @return An `OrderRequest` object populated with data from the message.
         * @throw std::runtime_error if the message is malformed or fails checksum validation.
         */
        static OrderRequest convert(const std::vector<char>& binaryData);
    };

} // namespace fix
