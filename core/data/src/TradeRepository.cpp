//
// Created by sujal on 27-10-2025.
//

#include "data/TradeRepository.h"

#include <iostream>
#include <thread>
#include "sqlite3.h"
#include "data/Constant.h"
#include "data/Query.h"

namespace data {
    TradeRepository::TradeRepository(const std::string &dbPath) : AsyncDatabaseRepository(dbPath) {
        initDatabase();
    }

    void TradeRepository::initDatabase() const {
        const SQLite::Database db(mDbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement query(db, data::query::createTradeTableQuery);
        query.exec();
    }

    void TradeRepository::addTrade(const common::Trade &trade) {
        enqueue([trade](const SQLite::Database &db) {
            try {
                SQLite::Statement query(db, data::query::insertIntoTradeTableQuery);
                query.bind(1, static_cast<sqlite3_int64>(trade.getTradeId()));
                query.bind(2, static_cast<sqlite3_int64>(trade.getBuyOrderId()));
                query.bind(3, static_cast<sqlite3_int64>(trade.getSellOrderId()));
                query.bind(4, static_cast<sqlite3_int64>(trade.getQty()));
                query.bind(5, trade.getPrice());
                const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    trade.getTimestamp().time_since_epoch()
                ).count();
                query.bind(6, ns);

                query.exec();
            } catch (const std::exception &e) {
                std::cerr << "Error adding new trade" << e.what() << "\n";
            }
        });
    }
} // data
