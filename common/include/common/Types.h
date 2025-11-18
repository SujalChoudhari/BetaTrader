//
// Created by sujal on 21-10-2025.
//

#pragma once
#include "Instrument.h"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace common {
    /**
     * @enum OrderSide
     * @brief Represents the side of an order (Buy or Sell).
     */
    enum class OrderSide : uint8_t {
        Buy, ///< A buy order.
        Sell, ///< a sell order.
    };

    /**
     * @enum OrderType
     * @brief Represents the execution logic of an order.
     */
    enum class OrderType : uint8_t {
        Limit, ///< A limit order, which is executed at a specified price or
               ///< better.
        Market, ///< A market order, which is executed immediately at the best
                ///< available price.
        Stop, ///< Triggers a market order once a set price is reached (Not
              ///< implemented)
        StopLimit ///< Triggers a limit order once a set price is reached (Not
                  ///< implemented)
    };

    /**
     * @enum OrderStatus
     * @brief Represents the lifecycle of an order.
     */
    enum class OrderStatus : uint8_t {
        New, ///< The order has been created but not yet matched.
        PartiallyFilled, ///< The order has been partially matched.
        Filled, ///< The order has been fully matched.
        Cancelled, ///< The order has been cancelled before being fully matched.
        Rejected ///< The order was rejected by the system and never entered the
                 ///< matching engine.
    };

    /**
     * @enum TimeInForce
     * @brief Guides How long the order is in the system.
     */
    enum class TimeInForce : uint8_t {
        DAY, ///< Keep the order in the system for a day
        GTC, ///< Good till cancellation
        IOC, ///< Immediate or cancel
        FOK, ///< Fill or kill
        MOO, ///< Market on Open
        MOC, ///< Market on Close
    };

    // --- OrderSide Serialization ---
    constexpr std::array<std::string_view, 2> order_side_names
            = {"Buy", "Sell"};

    inline std::string to_string(OrderSide side)
    {
        return std::string(order_side_names[static_cast<size_t>(side)]);
    }

    inline OrderSide from_string_OrderSide(std::string_view name)
    {
        for (size_t i = 0; i < order_side_names.size(); ++i)
            if (order_side_names[i] == name) return static_cast<OrderSide>(i);
        throw std::invalid_argument("Unknown OrderSide name");
    }

    // --- OrderType Serialization ---
    constexpr std::array<std::string_view, 2> order_type_names
            = {"Limit", "Market"};

    inline std::string to_string(OrderType type)
    {
        return std::string(order_type_names[static_cast<size_t>(type)]);
    }

    inline OrderType from_string_OrderType(std::string_view name)
    {
        for (size_t i = 0; i < order_type_names.size(); ++i)
            if (order_type_names[i] == name) return static_cast<OrderType>(i);
        throw std::invalid_argument("Unknown OrderType name");
    }

    // --- OrderStatus Serialization ---
    constexpr std::array<std::string_view, 5> order_status_names
            = {"New", "PartiallyFilled", "Filled", "Cancelled", "Rejected"};

    inline std::string to_string(OrderStatus status)
    {
        return std::string(order_status_names[static_cast<size_t>(status)]);
    }

    inline OrderStatus from_string_OrderStatus(std::string_view name)
    {
        for (size_t i = 0; i < order_status_names.size(); ++i)
            if (order_status_names[i] == name)
                return static_cast<OrderStatus>(i);
        throw std::invalid_argument("Unknown OrderStatus name");
    }

    // --- TimeInForce Serialization ---
    constexpr std::array<std::string_view, 6> time_in_force_names
            = {"DAY", "GTC", "IOC", "FOK", "MOO", "MOC"};

    inline std::string to_string(TimeInForce tif)
    {
        return std::string(time_in_force_names[static_cast<size_t>(tif)]);
    }

    inline TimeInForce from_string_TimeInForce(std::string_view name)
    {
        for (size_t i = 0; i < time_in_force_names.size(); ++i)
            if (time_in_force_names[i] == name)
                return static_cast<TimeInForce>(i);
        throw std::invalid_argument("Unknown TimeInForce name");
    }

    /** @brief A type alias for price values. */
    using Price = double;
    /** @brief A type alias for quantity values. */
    using Quantity = uint64_t;
    /** @brief A type alias for unique order identifiers. */
    using OrderID = uint64_t;
    /** @brief A type alias for unique trade identifiers. */
    using TradeID = uint64_t;
    /** @brief A type alias for financial instrument symbols. */
    using Symbol = Instrument;
    /** @brief A type alias for client identifiers. */
    using ClientID = std::string;
    /** @brief A type alias for session identifiers. */
    using SessionID = uint64_t;
} // namespace common
