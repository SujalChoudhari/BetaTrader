//
// Created by sujal on 26-10-2025.
//
#include "data/TradeIDRepository.h"
#include "data/Query.h"
#include "sqlite3.h"
#include <iostream>
#include <future>

#include "data/DataRunBookDefinations.h"
#include "logging/Runbook.h"

namespace data {
    TradeIDRepository::TradeIDRepository(const std::string &dbPath)
        : AsyncDatabaseRepository(dbPath) {
        initDatabase();
    }

    void TradeIDRepository::initDatabase() {
        enqueue([](const SQLite::Database &db) {
            try {
                SQLite::Statement createStmt(db, query::createTradeIdTableQuery);
                createStmt.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA2, "Error in TradeIDRepository::initDatabase: {}", std::string_view(e.what()));
            }
        });
    }

    common::TradeID TradeIDRepository::getCurrentTradeID() {
        auto p = std::make_shared<std::promise<common::TradeID> >();
        std::future<common::TradeID> f = p->get_future();

        enqueue([p](const SQLite::Database &db) {
            try {
                if (SQLite::Statement stmt(db, query::getTradeIdQuery); stmt.executeStep()) {
                    p->set_value(static_cast<common::TradeID>(stmt.getColumn(0).getInt64()));
                } else {
                    p->set_value(0);
                }
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA3, "Error in getCurrentTradeID: {}", std::string_view(e.what()));
                p->set_exception(std::current_exception());
            }
        });

        return f.get();
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
                LOG_ERROR(errors::EDATA4, "TradeID update failed: {}", std::string_view(e.what()));
            }
        });
    }

    void TradeIDRepository::truncateTradeID() {
        enqueue([](const SQLite::Database &db) {
            try {
                SQLite::Statement stmt(db, query::truncateTradeIdQuery);
                stmt.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA5, "Error in truncateTradeID: {} ", std::string_view( e.what()));
            }
        });
    }
}
