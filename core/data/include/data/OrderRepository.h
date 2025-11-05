#pragma once
#include "data/DatabaseWorker.h"
#include "common/Order.h"
#include <vector>
#include <functional>

namespace data {
    class OrderRepository {
    public:
        explicit OrderRepository(DatabaseWorker* dbWorker);
        virtual ~OrderRepository() = default;

        virtual void initDatabase();
        virtual void saveOrder(const common::Order& order);
        virtual void loadOrdersForInstrument(common::Instrument instrument, std::function<void(std::vector<common::Order>)> callback);
        virtual void removeOrder(common::OrderID orderId);
        virtual void updateOrder(const common::Order& order);
    private:
        DatabaseWorker* mDb;
    };
}
