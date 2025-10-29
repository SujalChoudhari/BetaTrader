//
// Created by sujal on 26-10-2025.
//

#pragma once
#include <string>

namespace data::query {
    const std::string createTradeIdTableQuery =
            "CREATE TABLE IF NOT EXISTS trade_id (id INTEGER PRIMARY KEY, current_id INTEGER NOT NULL)";

    const std::string getTradeIdQuery = "SELECT current_id FROM trade_id WHERE id = 1";

    const std::string setTradeIdQuery =
            "INSERT INTO trade_id (id, current_id) VALUES (1, ?) "
            "ON CONFLICT(id) DO UPDATE SET current_id = MAX(excluded.current_id, current_id)";

    const std::string truncateTradeIdQuery = "DELETE FROM trade_id";


    const std::string createTradeTableQuery =
            "CREATE TABLE IF NOT EXISTS trade ( "
            "trade_id INTEGER PRIMARY KEY, "
            "symbol TEXT NOT NULL, "
            "buy_order_id INTEGER NOT NULL, "
            "sell_order_id INTEGER NOT NULL, "
            "quantity INTEGER NOT NULL, "
            "price REAL NOT NULL, "
            "timestamp_ns INTEGER NOT NULL "
            "); ";
    const std::string insertIntoTradeTableQuery =
            "INSERT INTO trade( "
            "trade_id, "
            "symbol, "
            "buy_order_id, "
            "sell_order_id, "
            "quantity, "
            "price, "
            "timestamp_ns "
            ") VALUES ( "
            "?, ?, ?, ?, ?, ?, ? "
            ");";
}
