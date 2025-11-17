#include "data/TradeIDRepository.h"
#include "data/DataRunBookDefinations.h"
#include "data/Query.h"
#include "logging/Runbook.h"
#include "sqlite3.h"
#include <future>

namespace data {
    TradeIDRepository::TradeIDRepository(DatabaseWorker* dbWorker)
        : mDb(dbWorker)
    {
        initDatabase();
    }

    void TradeIDRepository::initDatabase()
    {
        mDb->enqueue([](SQLite::Database& db) {
            try {
                SQLite::Statement createStmt(db,
                                             query::createTradeIdTableQuery);
                createStmt.exec();
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA2,
                          "Error in TradeIDRepository::initDatabase: {}",
                          std::string_view(e.what()));
            }
        });
    }

    void TradeIDRepository::getCurrentTradeID(
            std::function<void(common::TradeID)> callback)
    {
        mDb->enqueue([callback](SQLite::Database& db) {
            try {
                SQLite::Statement stmt(db, query::getTradeIdQuery);
                if (stmt.executeStep()) {
                    callback(static_cast<common::TradeID>(
                            stmt.getColumn(0).getInt64()));
                }
                else {
                    callback(0);
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA3, "Error in getCurrentTradeID: {}",
                          std::string_view(e.what()));
            }
        });
    }

    void TradeIDRepository::setCurrentTradeID(common::TradeID tradeID)
    {
        mDb->enqueue([tradeID](SQLite::Database& db) {
            try {
                SQLite::Statement selectStmt(db, query::getTradeIdQuery);
                common::TradeID current = 0;
                if (selectStmt.executeStep())
                    current = static_cast<common::TradeID>(
                            selectStmt.getColumn(0).getInt64());

                if (tradeID > current) {
                    SQLite::Statement updateStmt(db, query::setTradeIdQuery);
                    updateStmt.bind(1, static_cast<sqlite3_int64>(tradeID));
                    updateStmt.exec();
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA4, "TradeID update failed: {}",
                          std::string_view(e.what()));
            }
        });
    }

    void TradeIDRepository::truncateTradeID()
    {
        mDb->enqueue([](SQLite::Database& db) {
            try {
                SQLite::Statement stmt(db, query::truncateTradeIdQuery);
                stmt.exec();
            }
            catch (const std::exception& e) {
                LOG_ERROR(errors::EDATA5, "Error in truncateTradeID: {}",
                          std::string_view(e.what()));
            }
        });
    }
} // namespace data
