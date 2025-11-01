//
// Created by sujal on 27-10-2025.
//
#include "data/TradeRepository.h"

#include "sqlite3.h"
#include "data/Query.h"
#include "logging/Runbook.h"
#include "data/DataRunBookDefinations.h"

namespace data {
    TradeRepository::TradeRepository(DatabaseWorker* dbWorker)
        : mDb(dbWorker) {
        initDatabase();
    }

    void TradeRepository::initDatabase() {
        mDb->enqueue([](SQLite::Database &db) {
            try {
                SQLite::Statement query(db, data::query::createTradeTableQuery);
                query.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA6, "Error in TradeRepository::initDatabase: {}", std::string_view(e.what()));
            }
        });
    }

    void TradeRepository::addTrade(const common::Trade &trade) {
        mDb->enqueue([trade](SQLite::Database &db) {
            try {
                SQLite::Statement query(db, data::query::insertIntoTradeTableQuery);
                query.bind(1, static_cast<sqlite3_int64>(trade.getTradeId()));
                query.bind(2, std::string(common::to_string(trade.getOrderSymbol())));
                query.bind(3, static_cast<sqlite3_int64>(trade.getBuyOrderId()));
                query.bind(4, static_cast<sqlite3_int64>(trade.getSellOrderId()));
                query.bind(5, static_cast<sqlite3_int64>(trade.getQuantity()));
                query.bind(6, trade.getPrice());
                const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    trade.getTimestamp().time_since_epoch()).count();
                query.bind(7, ns);
                query.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA7, "Error in TradeRepository::addTrade: {}", std::string_view(e.what()));
            }
        });
    }
}
