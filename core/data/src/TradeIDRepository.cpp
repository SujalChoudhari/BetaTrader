//
// Created by sujal on 26-10-2025.
//

#include "data/TradeIDRepository.h"

#include <iostream>
#include <thread>

#include "data/Constant.h" // Assumed to define databasePath
#include "data/Query.h"
#include "sqlite3.h"

namespace data {
    TradeIDRepository::TradeIDRepository()
        : TradeIDRepository(databasePath) {
        // Delegates to the specific constructor
    }

    TradeIDRepository::TradeIDRepository(const std::string &dbPath)
        : mDatabase(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
        // Ensure table exists on construction
        initDatabase();
    }

    void TradeIDRepository::initDatabase() const {
        SQLite::Statement createStmt(mDatabase, query::createTradeIdTableQuery);
        createStmt.exec();
    }

    common::TradeID TradeIDRepository::getCurrentTradeID() const {
        SQLite::Statement stmt(mDatabase, query::getTradeIdQuery);

        if (stmt.executeStep()) {
            return static_cast<common::TradeID>(stmt.getColumn(0).getInt64());
        }

        return 0;
    }

    void TradeIDRepository::setCurrentTradeID(common::TradeID tradeID) const {
        std::thread([this, tradeID]() {
            try {
                SQLite::Statement selectStmt(mDatabase, query::getTradeIdQuery);
                common::TradeID current = 0;
                if (selectStmt.executeStep()) {
                    current = static_cast<common::TradeID>(selectStmt.getColumn(0).getInt64());
                }

                if (tradeID > current) {
                    SQLite::Statement updateStmt(mDatabase, query::setTradeIdQuery);
                    updateStmt.bind(1, static_cast<sqlite3_int64>(tradeID));
                    updateStmt.exec();
                }
            } catch (const std::exception &e) {
                std::cerr << "Failed to update TradeID asynchronously: " << e.what() << "\n";
            }
        }).detach();
    }

    void TradeIDRepository::truncateTradeID() const {
        SQLite::Statement stmt(mDatabase, query::truncateTradeIdQuery);
        stmt.exec();
    }
} // namespace data
