#include "orderbook/OrderBook.h"
#include <algorithm>

namespace orderbook {

    OrderBook::OrderBook(const std::string& symbol) : mSymbol(symbol) {}

    void OrderBook::handleSnapshot(const fix::MarketDataSnapshotFullRefresh& snapshot) {
        std::lock_guard<std::mutex> lock(mMutex);
        mBids.clear();
        mAsks.clear();

        for (const auto& entry : snapshot.entries) {
            if (entry.entryType == fix::MDEntryType::Bid) {
                mBids[entry.price] = entry.size;
            } else if (entry.entryType == fix::MDEntryType::Offer) {
                mAsks[entry.price] = entry.size;
            }
        }
    }

    void OrderBook::handleIncremental(const fix::MarketDataIncrementalRefresh& refresh) {
        std::lock_guard<std::mutex> lock(mMutex);

        for (const auto& entry : refresh.entries) {
            updateLevel((char)entry.entryType, entry.price, entry.size, (char)entry.updateAction);
        }
    }

    void OrderBook::updateLevel(char type, double price, uint64_t qty, char action) {
        if (type == '0') { // Bid
            if (action == '0' || action == '1') {
                mBids[price] = qty;
            } else if (action == '2') {
                mBids.erase(price);
            }
            if (action != '2' && qty == 0) mBids.erase(price);
        } else { // Offer
            if (action == '0' || action == '1') {
                mAsks[price] = qty;
            } else if (action == '2') {
                mAsks.erase(price);
            }
            if (action != '2' && qty == 0) mAsks.erase(price);
        }
    }

    OrderBook::Snapshot OrderBook::getUISnapshot(size_t maxDepth) const {
        std::lock_guard<std::mutex> lock(mMutex);
        Snapshot ss;
        ss.symbol = mSymbol;

        size_t count = 0;
        for (auto const& [price, qty] : mBids) {
            if (count >= maxDepth) break;
            ss.bids.push_back({price, qty});
            count++;
        }

        count = 0;
        for (auto const& [price, qty] : mAsks) {
            if (count >= maxDepth) break;
            ss.asks.push_back({price, qty});
            count++;
        }

        if (!ss.bids.empty() && !ss.asks.empty()) {
            ss.spread = ss.asks[0].price - ss.bids[0].price;
            ss.midPrice = (ss.asks[0].price + ss.bids[0].price) / 2.0;
        } else {
            ss.spread = 0;
            ss.midPrice = 0;
        }

        return ss;
    }

} // namespace orderbook
