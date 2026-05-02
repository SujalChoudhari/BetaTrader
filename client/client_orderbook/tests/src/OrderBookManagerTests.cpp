#include <gtest/gtest.h>
#include "client_orderbook/OrderBookManager.h"

using namespace client_orderbook;

TEST(OrderBookManagerTests, GetModelCreatesNew) {
    OrderBookManager manager;
    auto model = manager.getModel("EURUSD");
    ASSERT_NE(model, nullptr);
    EXPECT_EQ(model->getSymbol(), "EURUSD");
}

TEST(OrderBookManagerTests, GetModelReturnsExisting) {
    OrderBookManager manager;
    auto model1 = manager.getModel("EURUSD");
    auto model2 = manager.getModel("EURUSD");
    EXPECT_EQ(model1, model2);
}

TEST(OrderBookManagerTests, GetAllModels) {
    OrderBookManager manager;
    manager.getModel("EURUSD");
    manager.getModel("USDJPY");
    
    auto all = manager.getAllModels();
    EXPECT_EQ(all.size(), 2);
    
    bool foundEUR = false;
    bool foundJPY = false;
    for (auto& m : all) {
        if (m->getSymbol() == "EURUSD") foundEUR = true;
        if (m->getSymbol() == "USDJPY") foundJPY = true;
    }
    EXPECT_TRUE(foundEUR);
    EXPECT_TRUE(foundJPY);
}

TEST(OrderBookManagerTests, ThreadSafetyBasic) {
    OrderBookManager manager;
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&manager, i]() {
            manager.getModel("SYM" + std::to_string(i % 3));
        });
    }
    for (auto& t : threads) t.join();
    
    EXPECT_EQ(manager.getAllModels().size(), 3);
}
