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
    constexpr auto createTradeTableQuery
            = "CREATE TABLE IF NOT EXISTS trades (trade_id INTEGER PRIMARY "
              "KEY, symbol TEXT, buy_order_id INTEGER, sell_order_id INTEGER, "
              "quantity INTEGER, price REAL, timestamp INTEGER);";
    constexpr auto insertIntoTradeTableQuery
            = "INSERT INTO trades (trade_id, symbol, buy_order_id, "
              "sell_order_id, quantity, price, timestamp) VALUES (?, ?, ?, ?, "
              "?, ?, ?);";

    // Trade ID Table
    constexpr auto createTradeIdTableQuery
            = "CREATE TABLE IF NOT EXISTS trade_id (id INTEGER PRIMARY KEY);";
    constexpr auto getTradeIdQuery = "SELECT id FROM trade_id;";
    constexpr auto setTradeIdQuery
            = "INSERT OR REPLACE INTO trade_id (id) VALUES (?);";
    constexpr auto truncateTradeIdQuery = "DELETE FROM trade_id;";

    // Order Table
    constexpr auto createOrderTableQuery
            = "CREATE TABLE IF NOT EXISTS orders (core_order_id INTEGER "
              "PRIMARY "
              "KEY, client_order_id INTEGER, client_id TEXT, sender_comp_id "
              "TEXT, symbol TEXT, side TEXT, type TEXT, "
              "time_in_force TEXT, price REAL, original_quantity INTEGER, "
              "remaining_quantity INTEGER, status TEXT, timestamp INTEGER);";
    constexpr auto insertIntoOrderTableQuery
            = "INSERT INTO orders (core_order_id, client_order_id, client_id, "
              "sender_comp_id, symbol, side, type, "
              "time_in_force, price, original_quantity, remaining_quantity, "
              "status, timestamp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
              "?);";
    constexpr auto loadOrdersForInstrumentQuery
            = "SELECT core_order_id, client_order_id, client_id, "
              "sender_comp_id, symbol, side, type, time_in_force, "
              "price, original_quantity, remaining_quantity, status, timestamp "
              "FROM orders WHERE symbol = ? AND (status = 'New' OR status = "
              "'PartiallyFilled');";
    constexpr auto removeOrderQuery
            = "DELETE FROM orders WHERE core_order_id = ?;";
    constexpr auto updateOrderQuery = "UPDATE orders SET remaining_quantity = "
                                      "?, status = ? WHERE core_order_id = ?;";

    // Clients Table
    constexpr auto createClientTableQuery
            = "CREATE TABLE IF NOT EXISTS clients (sender_comp_id TEXT PRIMARY "
              "KEY, is_active INTEGER);";
    constexpr auto insertClientQuery
            = "INSERT OR IGNORE INTO clients (sender_comp_id, is_active) "
              "VALUES (?, ?);";
    constexpr auto loadClientsQuery
            = "SELECT sender_comp_id FROM clients WHERE is_active = 1;";
    constexpr auto truncateClientsQuery = "DELETE FROM clients;";

    // Sequence Table
    constexpr auto createSequenceTable
            = "CREATE TABLE IF NOT EXISTS FIX_Sequences ("
              "compId TEXT PRIMARY KEY NOT NULL,"
              "inSeqNum INTEGER NOT NULL,"
              "outSeqNum INTEGER NOT NULL"
              ");";
    constexpr auto getSequenceNumberQuery = "SELECT inSeqNum, outSeqNum FROM "
                                            "FIX_Sequences WHERE compId = ?";
    constexpr auto updateSequenceNumberQuery
            = "INSERT INTO FIX_Sequences (compId, "
              "inSeqNum, outSeqNum) "
              "VALUES (?, ?, ?) "
              "ON CONFLICT(compId) DO UPDATE SET "
              "inSeqNum=excluded.inSeqNum, "
              "outSeqNum=excluded.outSeqNum;";
} // namespace data::query
