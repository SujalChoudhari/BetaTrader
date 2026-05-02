#include <client_orderbook/OrderBookModel.h>
#include <cassert>
#include <iostream>

using namespace client_orderbook;

void testSnapshotApplication() {
    OrderBookModel book("EURUSD");
    
    std::vector<PriceLevel> bids = {
        {1.1000, 100000, 5},
        {1.0995, 50000, 2}
    };
    std::vector<PriceLevel> asks = {
        {1.1005, 100000, 4},
        {1.1010, 75000, 3}
    };
    
    book.applySnapshot(bids, asks);
    
    auto currentBids = book.getBids();
    auto currentAsks = book.getAsks();
    
    assert(currentBids.size() == 2);
    assert(currentAsks.size() == 2);
    assert(currentBids[0].price == 1.1000);
    assert(currentAsks[0].price == 1.1005);
    
    std::cout << "testSnapshotApplication passed!" << std::endl;
}

void testIncrementalUpdate() {
    OrderBookModel book("EURUSD");
    
    // Initial state
    book.applySnapshot({{1.1000, 100000, 5}}, {{1.1005, 100000, 4}});
    
    // Update Bid (Action 1: Update/Insert)
    book.updateLevel(true, 1.1000, 150000, 1);
    auto bids = book.getBids();
    assert(bids[0].quantity == 150000);
    
    // Delete Bid (Action 2: Delete)
    book.updateLevel(true, 1.1000, 0, 2);
    bids = book.getBids();
    assert(bids.empty());
    
    std::cout << "testIncrementalUpdate passed!" << std::endl;
}

int main() {
    testSnapshotApplication();
    testIncrementalUpdate();
    return 0;
}
