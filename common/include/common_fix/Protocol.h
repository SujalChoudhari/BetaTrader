/**
 * @file Protocol.h
 * @brief Defines constants and enumerations for the FIX protocol.
 *
 * This file contains various character and string constants used in the FIX
 * protocol, such as SOH delimiter, message types, order sides, order types, and
 * order statuses.
 */

#pragma once

#include <string_view>

namespace fix {
    /// @brief Start of Header (SOH) character, used as a field delimiter in FIX
    /// messages.
    constexpr char SOH = '\x01';
    /// @brief The BeginString field (Tag 8) for FIX protocol version.
    constexpr std::string_view FIX_BEGIN_STRING = "FIX.4.4";

    /// @brief Message Type (Tag 35) for New Order Single.
    constexpr char MSG_TYPE_NEW_ORDER_SINGLE = 'D';
    /// @brief Message Type (Tag 35) for Execution Report.
    constexpr char MSG_TYPE_EXECUTION_REPORT = '8';
    /// @brief Message Type (Tag 35) for Order Cancel Request.
    constexpr char MSG_TYPE_ORDER_CANCEL_REQUEST = 'F';
    /// @brief Message Type (Tag 35) for Order Cancel Replace Request.
    constexpr char MSG_TYPE_ORDER_CANCEL_REPLACE_REQUEST = 'G';
    /// @brief Message Type (Tag 35) for Market Data Request.
    constexpr char MSG_TYPE_MARKET_DATA_REQUEST = 'V';
    /// @brief Message Type (Tag 35) for Market Data Snapshot Full Refresh.
    constexpr char MSG_TYPE_MARKET_DATA_SNAPSHOT_FULL_REFRESH = 'W';
    /// @brief Message Type (Tag 35) for Market Data Incremental Refresh.
    constexpr char MSG_TYPE_MARKET_DATA_INCREMENTAL_REFRESH = 'X';
    /// @brief Message Type (Tag 35) for Reject.
    constexpr char MSG_TYPE_REJECT = '3';
    /// @brief Message Type (Tag 35) for Business Message Reject.
    constexpr char MSG_TYPE_BUSINESS_MESSAGE_REJECT = 'j';

    /// @brief Order Side (Tag 54) for Buy.
    constexpr char ORDER_SIDE_BUY = '1';
    /// @brief Order Side (Tag 54) for Sell.
    constexpr char ORDER_SIDE_SELL = '2';

    /// @brief Order Type (Tag 40) for Market order.
    constexpr char ORDER_TYPE_MARKET = '1';
    /// @brief Order Type (Tag 40) for Limit order.
    constexpr char ORDER_TYPE_LIMIT = '2';

    /// @brief Order Status (Tag 39) for New order.
    constexpr char ORDER_STATUS_NEW = '0';
    /// @brief Order Status (Tag 39) for Partially Filled order.
    constexpr char ORDER_STATUS_PARTIALLY_FILLED = '1';
    /// @brief Order Status (Tag 39) for Filled order.
    constexpr char ORDER_STATUS_FILLED = '2';
    /// @brief Order Status (Tag 39) for Canceled order.
    constexpr char ORDER_STATUS_CANCELED = '4';
    /// @brief Order Status (Tag 39) for Rejected order.
    constexpr char ORDER_STATUS_REJECTED = '8';

    /// @brief Time In Force (Tag 59) for Day order.
    constexpr char TIME_IN_FORCE_DAY = '0';
    /// @brief Time In Force (Tag 59) for Good Till Cancel (GTC) order.
    constexpr char TIME_IN_FORCE_GTC = '1';
    /// @brief Time In Force (Tag 59) for Immediate Or Cancel (IOC) order.
    constexpr char TIME_IN_FORCE_IOC = '2';
    /// @brief Time In Force (Tag 59) for Fill Or Kill (FOK) order.
    constexpr char TIME_IN_FORCE_FOK = '3';

    /// @brief Subscription Request Type (Tag 263) for Snapshot.
    constexpr char SUBSCRIPTION_REQUEST_TYPE_SNAPSHOT = '0';
    /// @brief Subscription Request Type (Tag 263) for Snapshot and Updates.
    constexpr char SUBSCRIPTION_REQUEST_TYPE_SNAPSHOT_AND_UPDATES = '1';
    /// @brief Subscription Request Type (Tag 263) for Unsubscribe.
    constexpr char SUBSCRIPTION_REQUEST_TYPE_UNSUBSCRIBE = '2';

    /// @brief Market Depth (Tag 264) for Full Book.
    constexpr int MARKET_DEPTH_FULL_BOOK = 0;
    /// @brief Market Depth (Tag 264) for Top of Book.
    constexpr int MARKET_DEPTH_TOP_OF_BOOK = 1;

    /**
     * @enum MDEntryType
     * @brief Defines the type of market data entry (Tag 269).
     */
    enum class MDEntryType : char { Bid = '0', Offer = '1', Trade = '2' };

    /**
     * @enum MDUpdateAction
     * @brief Defines the update action for a market data entry (Tag 279).
     */
    enum class MDUpdateAction : char { New = '0', Change = '1', Delete = '2' };

} // namespace fix
