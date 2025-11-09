#include "data/OrderRepository.h"
#include "data/Query.h"
#include "sqlite3.h"
#include "logging/Runbook.h"
#include "data/DataRunBookDefinations.h"
#include "common/Types.h"
#include "common/Time.h"

namespace data {
    OrderRepository::OrderRepository(DatabaseWorker *dbWorker)
        : mDb(dbWorker) {
        OrderRepository::initDatabase();
    }

    void OrderRepository::initDatabase() {
        mDb->enqueue([](SQLite::Database &db) {
            try {
                SQLite::Statement query(db, data::query::createOrderTableQuery);
                query.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA8, "Error in OrderRepository::initDatabase: {}", std::string_view(e.what()));
            }
        });
    }

    void OrderRepository::saveOrder(const common::Order &order) {
        mDb->enqueue([order](SQLite::Database &db) {
            try {
                SQLite::Statement query(db, data::query::insertIntoOrderTableQuery);
                query.bind(1, static_cast<sqlite3_int64>(order.getId()));
                query.bind(2, order.getClientId());
                query.bind(3, common::to_string(order.getSymbol()));
                query.bind(4, common::to_string(order.getSide()));
                query.bind(5, common::to_string(order.getOrderType()));
                query.bind(6, common::to_string(order.getTimeInForce()));
                query.bind(7, order.getPrice());
                query.bind(8, static_cast<sqlite3_int64>(order.getOriginalQuantity()));
                query.bind(9, static_cast<sqlite3_int64>(order.getRemainingQuantity()));
                query.bind(10, common::to_string(order.getStatus()));
                const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    order.getTimestamp().time_since_epoch()).count();
                query.bind(11, ns);
                query.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA9, "Error in OrderRepository::saveOrder: {}", std::string_view(e.what()));
            }
        });
    }

    void OrderRepository::loadOrdersForInstrument(common::Instrument instrument,
                                                  std::function<void(std::vector<common::Order>)> callback) {
        mDb->enqueue([instrument, callback](const SQLite::Database &db) {
            std::vector<common::Order> orders;
            try {
                SQLite::Statement query(db, query::loadOrdersForInstrumentQuery);
                query.bind(1, common::to_string(instrument));

                while (query.executeStep()) {
                    const common::OrderID orderId = static_cast<common::OrderID>(query.getColumn(0).getInt64());
                    const common::ClientID clientId = query.getColumn(1).getText();
                    const common::Instrument symbol = common::from_string(query.getColumn(2).getText());
                    const common::OrderSide side = common::from_string_OrderSide(query.getColumn(3).getText());
                    const common::OrderType type = common::from_string_OrderType(query.getColumn(4).getText());
                    const common::TimeInForce timeInForce = common::from_string_TimeInForce(query.getColumn(5).getText());
                    const common::Price price = query.getColumn(6).getDouble();
                    const common::Quantity originalQuantity = static_cast<common::Quantity>(query.getColumn(7).
                        getInt64());
                    const common::Quantity remainingQuantity = static_cast<common::Quantity>(query.getColumn(8).
                        getInt64());
                    const common::OrderStatus status = common::from_string_OrderStatus(query.getColumn(9).getText());

                    auto ns_duration = std::chrono::nanoseconds(query.getColumn(10).getInt64());
                    const common::Timestamp timestamp(
                        std::chrono::duration_cast<common::Timestamp::duration>(ns_duration));

                    common::Order order(orderId, symbol, clientId, side, type, timeInForce, originalQuantity, price, timestamp);
                    order.setRemainingQuantity(remainingQuantity);
                    order.setStatus(status);
                    orders.push_back(order);
                }
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA10, "Error in OrderRepository::loadOrdersForInstrument: {}",
                          std::string_view(e.what()));
            }
            callback(orders);
        });
    }

    void OrderRepository::removeOrder(common::OrderID orderId) {
        mDb->enqueue([orderId](const SQLite::Database &db) {
            try {
                SQLite::Statement query(db, query::removeOrderQuery);
                query.bind(1, static_cast<sqlite3_int64>(orderId));
                query.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA11, "Error in OrderRepository::removeOrder: {}", std::string_view(e.what()));
            }
        });
    }

    void OrderRepository::updateOrder(const common::Order &order) {
        mDb->enqueue([order](SQLite::Database &db) {
            try {
                SQLite::Statement query(db, data::query::updateOrderQuery);
                query.bind(1, static_cast<sqlite3_int64>(order.getRemainingQuantity()));
                query.bind(2, common::to_string(order.getStatus()));
                query.bind(3, static_cast<sqlite3_int64>(order.getId()));
                query.exec();
            } catch (const std::exception &e) {
                LOG_ERROR(errors::EDATA12, "Error in OrderRepository::updateOrder: {}", std::string_view(e.what()));
            }
        });
    }
}
