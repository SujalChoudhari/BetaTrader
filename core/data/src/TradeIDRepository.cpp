//
// Created by sujal on 26-10-2025.
//
#include "data/TradeIDRepository.h"
#include "data/Query.h"
#include "sqlite3.h"
#include <iostream>

namespace data {
    TradeIDRepository::TradeIDRepository(const std::string &dbPath)
        : AsyncDatabaseRepository(dbPath) {
        initDatabase();
    }

    void TradeIDRepository::initDatabase() const {
        const SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement createStmt(db, query::createTradeIdTableQuery);
        createStmt.exec();
    }

    common::TradeID TradeIDRepository::getCurrentTradeID() const {
        const SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        if (SQLite::Statement stmt(db, query::getTradeIdQuery); stmt.executeStep())
            return static_cast<common::TradeID>(stmt.getColumn(0).getInt64());
        return 0;
    }

    void TradeIDRepository::setCurrentTradeID(common::TradeID tradeID) {
        enqueue([tradeID](const SQLite::Database &db) {
            try {
                SQLite::Statement selectStmt(db, query::getTradeIdQuery);
                common::TradeID current = 0;
                if (selectStmt.executeStep())
                    current = static_cast<common::TradeID>(selectStmt.getColumn(0).getInt64());

                if (tradeID > current) {
                    SQLite::Statement updateStmt(db, query::setTradeIdQuery);
                    updateStmt.bind(1, static_cast<sqlite3_int64>(tradeID));
                    updateStmt.exec();
                }
            } catch (const std::exception &e) {
                std::cerr << "TradeID update failed: " << e.what() << "\n";
            }
        });
    }

    void TradeIDRepository::truncateTradeID() const {
        const SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement stmt(db, query::truncateTradeIdQuery);
        stmt.exec();
    }
}
