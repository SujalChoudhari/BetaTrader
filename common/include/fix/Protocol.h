#pragma once

#include <string_view>

namespace fix {
    // SOH character
    constexpr char SOH = '\x01';
    // BeginString
    constexpr std::string_view FIX_BEGIN_STRING = "FIX.4.4";
    // MsgType
    constexpr char MSG_TYPE_NEW_ORDER_SINGLE = 'D';
    constexpr char MSG_TYPE_EXECUTION_REPORT = '8';
    // OrderSide
    constexpr char ORDER_SIDE_BUY = '1';
    constexpr char ORDER_SIDE_SELL = '2';
    // OrderType
    constexpr char ORDER_TYPE_MARKET = '1';
    constexpr char ORDER_TYPE_LIMIT = '2';
    // OrderStatus
    constexpr char ORDER_STATUS_NEW = '0';
    constexpr char ORDER_STATUS_PARTIALLY_FILLED = '1';
    constexpr char ORDER_STATUS_FILLED = '2';
    constexpr char ORDER_STATUS_CANCELED = '4';
    constexpr char ORDER_STATUS_REJECTED = '8';
    // TimeInForce
    constexpr char TIME_IN_FORCE_DAY = '0';
    constexpr char TIME_IN_FORCE_GTC = '1';
    constexpr char TIME_IN_FORCE_IOC = '2';
    constexpr char TIME_IN_FORCE_FOK = '3';

} // namespace fix
