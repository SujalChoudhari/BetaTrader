#pragma once

#include "common/Types.h"
#include "common_fix/MarketDataSnapshotFullRefresh.h"
#include "common_fix/MarketDataIncrementalRefresh.h"
#include <map>
#include <mutex>
#include <vector>
#include <string>

namespace orderbook {

    struct Level {
        double price;
        uint64_t qty;
    };

    /**
     * @class OrderBook
     * @brief A client-side representation of the exchange's order book.
     * 
     * Maintains a sorted Bid/Ask ladder using snapshots and incremental updates.
     */
    class OrderBook {
    public:
        OrderBook(const std::string& symbol);
        ~OrderBook() = default;

        void handleSnapshot(const fix::MarketDataSnapshotFullRefresh& snapshot);
        void handleIncremental(const fix::MarketDataIncrementalRefresh& refresh);

        // UI Helpers
        struct Snapshot {
            std::string symbol;
            std::vector<Level> bids;
            std::vector<Level> asks;
            double spread;
            double midPrice;
        };

        Snapshot getUISnapshot(size_t maxDepth = 10) const;

        const std::string& getSymbol() const { return mSymbol; }

    private:
        std::string mSymbol;
        mutable std::mutex mMutex;

        // price -> qty
        std::map<double, uint64_t, std::greater<double>> mBids;
        std::map<double, uint64_t, std::less<double>> mAsks;

        void updateLevel(char type, double price, uint64_t qty, char action);
    };

} // namespace orderbook
