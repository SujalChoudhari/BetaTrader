#pragma once

#include <map>
#include <vector>
#include <shared_mutex>
#include <optional>
#include <string>

namespace client_orderbook {

/**
 * @struct PriceLevel
 * @brief Represents aggregated order volume at a specific price.
 */
struct PriceLevel {
    double price;
    uint64_t quantity;
    int numOrders;
};

/**
 * @class OrderBookModel
 * @brief Thread-safe container for Level 2 market depth.
 */
class OrderBookModel {
public:
    OrderBookModel(const std::string& symbol);

    /**
     * @brief Updates the book from a full snapshot (35=W).
     */
    void applySnapshot(const std::vector<PriceLevel>& bids, 
                       const std::vector<PriceLevel>& asks);

    /**
     * @brief Applies a single incremental update (35=X).
     */
    void updateLevel(bool isBid, double price, uint64_t quantity, int action);

    /**
     * @brief Returns a snapshot of the bids for rendering.
     */
    std::vector<PriceLevel> getBids(size_t depth = 20) const;

    /**
     * @brief Returns a snapshot of the asks for rendering.
     */
    std::vector<PriceLevel> getAsks(size_t depth = 20) const;

    /**
     * @brief Returns the Best Bid and Best Offer.
     */
    std::pair<std::optional<PriceLevel>, std::optional<PriceLevel>> getTopOfBook() const;

    std::string getSymbol() const { return mSymbol; }

private:
    std::string mSymbol;
    std::map<double, PriceLevel, std::greater<double>> mBids;
    std::map<double, PriceLevel, std::less<double>> mAsks;
    mutable std::shared_mutex mMutex;
};

} // namespace client_orderbook
