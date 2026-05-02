#include <client_orderbook/OrderBookModel.h>
#include <mutex>

namespace client_orderbook {

OrderBookModel::OrderBookModel(const std::string& symbol) : mSymbol(symbol) {}

void OrderBookModel::applySnapshot(const std::vector<PriceLevel>& bids, 
                                   const std::vector<PriceLevel>& asks) {
    std::unique_lock<std::shared_mutex> lock(mMutex);
    mBids.clear();
    mAsks.clear();
    for (const auto& pl : bids) {
        mBids[pl.price] = pl;
    }
    for (const auto& pl : asks) {
        mAsks[pl.price] = pl;
    }
}

void OrderBookModel::updateLevel(bool isBid, double price, uint64_t quantity, int action) {
    std::unique_lock<std::shared_mutex> lock(mMutex);
    
    auto handleUpdate = [&](auto& book) {
        if (action == 2) { // Delete
            book.erase(price);
        } else { // New (0) or Update (1)
            if (quantity == 0) {
                book.erase(price);
            } else {
                book[price] = {price, quantity, 1}; // Simplification: numOrders set to 1
            }
        }
    };

    if (isBid) handleUpdate(mBids);
    else handleUpdate(mAsks);
}

std::vector<PriceLevel> OrderBookModel::getBids(size_t depth) const {
    std::shared_lock lock(mMutex);
    std::vector<PriceLevel> result;
    size_t count = 0;
    for (const auto& [price, pl] : mBids) {
        if (count++ >= depth) break;
        result.push_back(pl);
    }
    return result;
}

std::vector<PriceLevel> OrderBookModel::getAsks(size_t depth) const {
    std::shared_lock lock(mMutex);
    std::vector<PriceLevel> result;
    size_t count = 0;
    for (const auto& [price, pl] : mAsks) {
        if (count++ >= depth) break;
        result.push_back(pl);
    }
    return result;
}

std::pair<std::optional<PriceLevel>, std::optional<PriceLevel>> OrderBookModel::getTopOfBook() const {
    std::shared_lock lock(mMutex);
    std::optional<PriceLevel> bestBid;
    std::optional<PriceLevel> bestAsk;

    if (!mBids.empty()) bestBid = mBids.begin()->second;
    if (!mAsks.empty()) bestAsk = mAsks.begin()->second;

    return {bestBid, bestAsk};
}

} // namespace client_orderbook
