#include "ohlc/CandleAggregator.h"
#include <chrono>

namespace ohlc {

    CandleAggregator::CandleAggregator(MarketHistoryRepository& repo) : mRepo(repo) {}

    void CandleAggregator::onTrade(const std::string& symbol, double price, uint64_t qty, int64_t timestampNs) {
        std::lock_guard<std::mutex> lock(mMutex);
        
        // Update 1m and 5m intervals
        updateAggregate(mAggregates[symbol][1], 1, symbol, price, qty, timestampNs);
        updateAggregate(mAggregates[symbol][5], 5, symbol, price, qty, timestampNs);
    }

    void CandleAggregator::updateAggregate(Aggregate& agg, int interval, const std::string& symbol, double price, uint64_t qty, int64_t timestampNs) {
        int64_t timestampSec = timestampNs / 1000000000LL;
        int64_t bucketStart = (timestampSec / (interval * 60)) * (interval * 60);

        if (!agg.active || bucketStart > agg.current.timestamp) {
            // Finalize old candle if it exists
            if (agg.active) {
                mRepo.addCandle(interval, agg.current);
                if (mCallback) mCallback(interval, agg.current);
            }

            // Initialize new candle
            agg.current.symbol = symbol;
            agg.current.timestamp = bucketStart;
            agg.current.open = price;
            agg.current.high = price;
            agg.current.low = price;
            agg.current.close = price;
            agg.current.volume = qty;
            agg.active = true;
        } else {
            // Update existing candle
            agg.current.high = std::max(agg.current.high, price);
            agg.current.low = std::min(agg.current.low, price);
            agg.current.close = price;
            agg.current.volume += qty;
        }
    }

} // namespace ohlc
