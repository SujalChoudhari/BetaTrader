/**
 * @file Query.h
 * @brief SQL query string constants for the data layer.
 *
 * This header defines the SQL statements used to create and access
 * persistent tables for trades, orders, and trade id tracking.
 */

#pragma once

namespace data::query {
    // Trade Table
    constexpr auto createTradeTableQuery = "CREATE TABLE IF NOT EXISTS trades (trade_id INTEGER PRIMARY KEY, symbol TEXT, buy_order_id INTEGER, sell_order_id INTEGER, quantity INTEGER, price REAL, timestamp INTEGER);";
    constexpr auto insertIntoTradeTableQuery = "INSERT INTO trades (trade_id, symbol, buy_order_id, sell_order_id, quantity, price, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?);";

    // Trade ID Table
    constexpr auto createTradeIdTableQuery = "CREATE TABLE IF NOT EXISTS trade_id (id INTEGER PRIMARY KEY);";
    constexpr auto getTradeIdQuery = "SELECT id FROM trade_id;";
    constexpr auto setTradeIdQuery = "INSERT OR REPLACE INTO trade_id (id) VALUES (?);";
    constexpr auto truncateTradeIdQuery = "DELETE FROM trade_id;";

    // Order Table
    constexpr auto createOrderTableQuery = "CREATE TABLE IF NOT EXISTS orders (order_id INTEGER PRIMARY KEY, client_id INTEGER, symbol TEXT, side TEXT, type TEXT, price REAL, original_quantity INTEGER, remaining_quantity INTEGER, status TEXT, timestamp INTEGER);";
    constexpr auto insertIntoOrderTableQuery = "INSERT INTO orders (order_id, client_id, symbol, side, type, price, original_quantity, remaining_quantity, status, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    constexpr auto loadOrdersForInstrumentQuery = "SELECT * FROM orders WHERE symbol = ?;";
    constexpr auto removeOrderQuery = "DELETE FROM orders WHERE order_id = ?;";
    constexpr auto updateOrderQuery = "UPDATE orders SET remaining_quantity = ?, status = ? WHERE order_id = ?;";
}
