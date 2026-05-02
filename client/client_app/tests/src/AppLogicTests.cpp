#include <gtest/gtest.h>
#include "App.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "common_fix/MarketDataIncrementalRefresh.h"

namespace client_app {

class AppLogicTests : public ::testing::Test {
protected:
    App app;
};

TEST_F(AppLogicTests, HandleMarketDataSnapshot) {
    fix::MarketDataSnapshotFullRefresh snapshot;
    snapshot.symbol = common::Instrument::EURUSD;
    
    fix::MarketDataEntry entry;
    entry.entryType = fix::MDEntryType::Bid;
    entry.price = 1.1000;
    entry.size = 100000;
    snapshot.entries.push_back(entry);
    
    app.onFixMessage(snapshot);
    
    auto model = app.getOrderBookManager().getModel("EURUSD");
    auto bids = model->getBids();
    ASSERT_EQ(bids.size(), 1);
    EXPECT_EQ(bids[0].price, 1.1000);
    EXPECT_EQ(bids[0].quantity, 100000);
}

TEST_F(AppLogicTests, HandleMarketDataIncremental) {
    // First seed with a snapshot
    fix::MarketDataSnapshotFullRefresh snapshot;
    snapshot.symbol = common::Instrument::EURUSD;
    snapshot.entries.push_back({fix::MDEntryType::Bid, 1.1000, 100000});
    app.onFixMessage(snapshot);
    
    // Then incremental update
    fix::MarketDataIncrementalRefresh refresh;
    refresh.symbol = common::Instrument::EURUSD;
    
    fix::MarketDataIncrementalEntry entry;
    entry.entryType = fix::MDEntryType::Bid;
    entry.price = 1.1000;
    entry.size = 150000;
    entry.updateAction = fix::MDUpdateAction::Change;
    refresh.entries.push_back(entry);
    
    app.onFixMessage(refresh);
    
    auto model = app.getOrderBookManager().getModel("EURUSD");
    auto bids = model->getBids();
    ASSERT_EQ(bids.size(), 1);
    EXPECT_EQ(bids[0].price, 1.1000);
    EXPECT_EQ(bids[0].quantity, 150000);
}

} // namespace client_app
