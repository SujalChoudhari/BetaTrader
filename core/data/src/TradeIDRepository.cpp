#include "data/TradeIDRepository.h"
#include "data/Query.h"
#include "sqlite3.h"
#include <future>
#include "data/DataRunBookDefinations.h"
#include "logging/Runbook.h"

namespace data {
    TradeIDRepository::TradeIDRepository(DatabaseWorker &dbWorker)
        : mDb(dbWorker) {
        initDatabase();
    }

    TradeIDRepository::TradeIDRepository(std::shared_ptr<data::DatabaseWorker> dbWorker)
        : mDb(*dbWorker) {
        initDatabase();
    }

    void TradeIDRepository::initDatabase() {
        mDb.enqueue([](SQLite::Database &db) {
            try {
                SQLite::Statement createStmt(db, query::createTradeIdTableQuery);
                createStmt.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA2,
                          "Error in TradeIDRepository::initDatabase: {}",
                          std::string_view(e.what()));
            }
        });
    }

    common::TradeID TradeIDRepository::getCurrentTradeID() {
        auto promisePtr = std::make_shared<std::promise<common::TradeID> >();
        std::future<common::TradeID> future = promisePtr->get_future();

        mDb.enqueue([promisePtr](SQLite::Database &db) {
            try {
                SQLite::Statement stmt(db, query::getTradeIdQuery);
                if (stmt.executeStep()) {
                    promisePtr->set_value(
                        static_cast<common::TradeID>(stmt.getColumn(0).getInt64()));
                } else {
                    promisePtr->set_value(0);
                }
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA3,
                          "Error in getCurrentTradeID: {}",
                          std::string_view(e.what()));
                promisePtr->set_exception(std::current_exception());
            }
        });

        return future.get();
    }

    void TradeIDRepository::setCurrentTradeID(common::TradeID tradeID) {
        mDb.enqueue([tradeID](SQLite::Database &db) {
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
                LOG_ERROR(errors::EDATA4,
                          "TradeID update failed: {}",
                          std::string_view(e.what()));
            }
        });
    }

    void TradeIDRepository::truncateTradeID() {
        mDb.enqueue([](SQLite::Database &db) {
            try {
                SQLite::Statement stmt(db, query::truncateTradeIdQuery);
                stmt.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA5,
                          "Error in truncateTradeID: {}",
                          std::string_view(e.what()));
            }
        });
    }
}
