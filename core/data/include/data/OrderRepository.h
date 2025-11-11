/**
 * @file OrderRepository.h
 * @brief Repository interface for persisting and loading orders.
 *
 * Provides an abstraction over the underlying database worker for storing,
 * retrieving and updating order records.
 */

#pragma once
#include "common/Order.h"
#include "data/DatabaseWorker.h"
#include <functional>
#include <vector>

namespace data {
    /**
     * @class OrderRepository
     * @brief Persists and retrieves `common::Order` objects.
     */
    class OrderRepository {
    public:
        explicit OrderRepository(DatabaseWorker* dbWorker);
        virtual ~OrderRepository() = default;

        virtual void initDatabase();
        virtual void saveOrder(const common::Order& order);
        virtual void loadOrdersForInstrument(
                common::Instrument instrument,
                std::function<void(std::vector<common::Order>)> callback);
        virtual void removeOrder(common::OrderID orderId);
        virtual void updateOrder(const common::Order& order);

    private:
        DatabaseWorker* mDb;
    };
} // namespace data
