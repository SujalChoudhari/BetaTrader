//
// Created by sujal on 26-10-2025.
//
#include "data/TradeIDRepository.h"
#include "data/Query.h"
#include "sqlite3.h"
#include <iostream>
#include <future>

namespace data {
    TradeIDRepository::TradeIDRepository(const std::string &dbPath)
        : AsyncDatabaseRepository(dbPath) {
        initDatabase();
    }

    void TradeIDRepository::initDatabase() {
        enqueue([](SQLite::Database &db) {
            try {
                SQLite::Statement createStmt(db, query::createTradeIdTableQuery);
                createStmt.exec();
            } catch (const std::exception &e) {
                std::cerr << "Error in TradeIDRepository::initDatabase: " << e.what() << "\n";
            }
        });
    }

    common::TradeID TradeIDRepository::getCurrentTradeID() {
        auto p = std::make_shared<std::promise<common::TradeID>>();
        std::future<common::TradeID> f = p->get_future();

        enqueue([p](SQLite::Database &db) {
            try {
                if (SQLite::Statement stmt(db, query::getTradeIdQuery); stmt.executeStep()) {
                    p->set_value(static_cast<common::TradeID>(stmt.getColumn(0).getInt64()));
                } else {
                    p->set_value(0);
                }
            } catch (const std::exception &e) {
                std::cerr << "Error in getCurrentTradeID: " << e.what() << "\n";
                p->set_exception(std::current_exception());
            }
        });

        return f.get();
    }

    void TradeIDRepository::setCurrentTradeID(common::TradeID tradeID) {
        enqueue([tradeID](SQLite::Database &db) {
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

    void TradeIDRepository::truncateTradeID() {
        enqueue([](SQLite::Database &db) {
            try {
                SQLite::Statement stmt(db, query::truncateTradeIdQuery);
                stmt.exec();
            } catch (const std::exception &e) {
                std::cerr << "Error in truncateTradeID: " << e.what() << "\n";
            }
        });
    }
}

