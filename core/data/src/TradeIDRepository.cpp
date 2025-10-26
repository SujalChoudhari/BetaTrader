//
// Created by sujal on 26-10-2025.
//

#include "data/TradeIDRepository.h"
#include "data/Constant.h" // Assumed to define databasePath
#include "data/Query.h"
#include "sqlite3.h"

namespace data {

    TradeIDRepository::TradeIDRepository()
        : TradeIDRepository(databasePath) {
        // Delegates to the specific constructor
    }

    TradeIDRepository::TradeIDRepository(const std::string& dbPath)
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

        // executeStep() returns true if a row is returned
        if (stmt.executeStep()) {
            return static_cast<common::TradeID>(stmt.getColumn(0).getInt64());
        }

        // No row found (table is empty), return 0
        return 0;
    }

    void TradeIDRepository::setCurrentTradeID(common::TradeID tradeID) const {
        SQLite::Statement stmt(mDatabase, query::setTradeIdQuery);
        stmt.bind(1, static_cast<sqlite3_int64>(tradeID));
        stmt.exec();
    }

    void TradeIDRepository::truncateTradeID() const {
        SQLite::Statement stmt(mDatabase, query::truncateTradeIdQuery);
        stmt.exec();
    }

} // namespace data
