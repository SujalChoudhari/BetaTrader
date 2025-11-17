#pragma once

#include <string>
#include "fix/Types.h"     // For fix::ClientOrderID (though using std::string for now)
#include "fix/Protocol.h"  // For order status constants

namespace fix {

/**
 * @brief Represents a FIX Order Response message.
 *
 * This struct encapsulates the common fields returned in response to an order
 * request (e.g., New Order Single, Order Cancel Request, Order Cancel Replace Request).
 */
struct OrderResponse {
    /** @brief Exchange-assigned Order ID (OrderID, FIX Tag 37). */
    std::string orderID;
    /** @brief Client-assigned Order ID (ClOrdID, FIX Tag 11).
     *         Corresponds to the ClOrdID sent in the original request. */
    std::string clOrdID;
    /** @brief Current status of the order (OrdStatus, FIX Tag 39).
     *         e.g., '0' for New, '8' for Rejected. */
    char ordStatus;
    /** @brief Free format text, often used for error messages or additional information (Text, FIX Tag 58). */
    std::string text;

    /**
     * @brief Constructs a new OrderResponse object.
     * @param orderID The exchange-assigned order ID.
     * @param clOrdID The client-assigned order ID.
     * @param ordStatus The status of the order.
     * @param text Additional text or error message. Defaults to an empty string.
     */
    OrderResponse(
        const std::string& orderID,
        const std::string& clOrdID,
        char ordStatus,
        const std::string& text = ""
    ) : orderID(orderID), clOrdID(clOrdID), ordStatus(ordStatus), text(text) {}
};

} // namespace fix
