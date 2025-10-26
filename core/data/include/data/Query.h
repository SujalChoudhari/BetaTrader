//
// Created by sujal on 26-10-2025.
//

#pragma once
#include <string>

namespace data::query {
    const std::string createTradeIdTableQuery =
            "CREATE TABLE IF NOT EXISTS trade_id (id INTEGER PRIMARY KEY, current_id INTEGER NOT NULL)";

    const std::string getTradeIdQuery = "SELECT current_id FROM trade_id WHERE id = 1";

    const std::string setTradeIdQuery = "INSERT OR REPLACE INTO trade_id (id, current_id) VALUES (1, ?)";

    const std::string truncateTradeIdQuery = "DELETE FROM trade_id";
}
