#pragma once

#include "data/DatabaseWorker.h"
#include "data/OrderRepository.h"
#include "data/TradeRepository.h"
#include "trading_core/OrderManager.h"
#include "trading_core/OrderBook.h"
#include "trading_core/Matcher.h"
#include "trading_core/RiskManager.h"
#include "trading_core/TradeIDGenerator.h"
#include <gmock/gmock.h> // Include gmock for MOCK_METHOD

class MockDatabaseWorker : public data::DatabaseWorker {
public:
    MockDatabaseWorker() : data::DatabaseWorker() {}

    void enqueue(const std::function<void(SQLite::Database&)> task) override {
        task(*static_cast<SQLite::Database*>(nullptr));
    }
};

// Mock for OrderRepository
class MockOrderRepository : public data::OrderRepository {
public:
    explicit MockOrderRepository(data::DatabaseWorker* dbWorker) : data::OrderRepository(dbWorker) {}
    void initDatabase() override {}
    mutable int saveOrderCallCount = 0;
    mutable int updateOrderCallCount = 0;
    mutable int removeOrderCallCount = 0;

    void saveOrder(const common::Order& order) override { saveOrderCallCount++; }
    void updateOrder(const common::Order& order) override { updateOrderCallCount++; }
    void removeOrder(common::OrderID orderId) override { removeOrderCallCount++; }

    void loadOrdersForInstrument(common::Instrument symbol, std::function<void(std::vector<common::Order>)> callback) override {
        callback({}); // Immediately call the callback with an empty vector
    }
};

// Mock for TradeRepository
class MockTradeRepository : public data::TradeRepository {
public:
    explicit MockTradeRepository(data::DatabaseWorker* dbWorker) : data::TradeRepository(dbWorker) {}
    void initDatabase() override {}
};

// Mock for OrderManager
class MockOrderManager : public trading_core::OrderManager {
public:
    mutable int addOrderCallCount = 0;
    mutable int getOrderByIdCallCount = 0;
    mutable int removeOrderByIdCallCount = 0;
    
    std::unordered_map<common::OrderID, common::Order*> orders;
    std::vector<std::unique_ptr<common::Order>> ownedOrders;

    bool addOrder(std::unique_ptr<common::Order> order) override {
        addOrderCallCount++;
        orders[order->getId()] = order.get();
        ownedOrders.push_back(std::move(order));
        return true;
    }
    std::optional<common::Order*> getOrderById(const common::OrderID &id) const override {
        getOrderByIdCallCount++;
        if (orders.count(id)) {
            return orders.at(id);
        }
        return std::nullopt;
    }
    bool removeOrderById(const common::OrderID &id) override {
        removeOrderByIdCallCount++;
        if(orders.count(id)) {
            orders.erase(id);
        }
        return true;
    }
};

// Mock for OrderBook
class MockOrderBook : public trading_core::OrderBook {
public:
    mutable int insertOrderCallCount = 0;
    mutable int cancelOrderCallCount = 0;
    bool cancelOrderResult = true;

    void insertOrder(common::Order* order) override { insertOrderCallCount++; }
    bool cancelOrder(const common::OrderID &orderId) override {
        cancelOrderCallCount++;
        return cancelOrderResult;
    }

    // Mock the virtual methods from OrderBook.h
    MOCK_METHOD(BidMap*, getBidMap, (), (override));
    MOCK_METHOD(AskMap*, getAskMap, (), (override));
};

// Mock for Matcher
class MockMatcher : public trading_core::Matcher {
public:
    MockMatcher() : trading_core::Matcher(nullptr) {}
    mutable int matchCallCount = 0;
    std::vector<common::Trade> matchResult;

    std::vector<common::Trade> match(common::Order* incomingOrder, trading_core::OrderBook &orderBook) override {
        matchCallCount++;
        return matchResult;
    }
};

// Mock for RiskManager
class MockRiskManager : public trading_core::RiskManager {
public:
    MockRiskManager() : trading_core::RiskManager(nullptr) {}
    mutable int preCheckCallCount = 0;
    mutable int postTradeUpdateCallCount = 0;
    bool preCheckResult = true;

    bool preCheck(const common::Order &order, trading_core::OrderBook& orderBook) override {
        preCheckCallCount++;
        return preCheckResult;
    }
    void postTradeUpdate(const common::Trade &trade) override { postTradeUpdateCallCount++; }
};

// Mock for TradeIDGenerator
class MockTradeIDGenerator : public trading_core::TradeIDGenerator {
public:
    explicit MockTradeIDGenerator(data::DatabaseWorker* dbWorker) : trading_core::TradeIDGenerator(dbWorker) {}
    void saveState() override {}
    void loadState() override {}
    common::TradeID nextId() override { return 1; }
};
