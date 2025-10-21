//
// Created by sujal on 21-10-2025.
//

#pragma once

namespace trading_core {
    /**
     * Represents the types of commands that are possible
     *
     * 1. New Order - when trader wants to put in a new order
     * 2. Cancel Order - when an existing offer is to be canceled
     * 3. Modify Order - Update an existing offer.
     */
    enum class CommandType {
        NewOrder,
        CancelOrder,
        ModifyOrder
    };
}
