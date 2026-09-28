#include "ohlc/CandleAggregator.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

namespace ohlc {

    CandleAggregator::CandleAggregator(MarketHistoryRepository& repo) : mRepo(repo) {}

    void CandleAggregator::onTrade(const std::string& symbol, double price, uint64_t qty, int64_t timestampNs) {
        std::lock_guard<std::mutex> lock(mMutex);
        
        // Update 1m and 5m intervals
        updateAggregate(mAggregates[symbol][1], 1, symbol, price, qty, timestampNs);
        updateAggregate(mAggregates[symbol][5], 5, symbol, price, qty, timestampNs);
    }

    void CandleAggregator::seedHistoricalData(const std::string& symbol,
                                               int interval,
                                               size_t candleCount,
                                               int64_t nowNs,
                                               double startingPrice,
                                               uint32_t seed) {
        if (candleCount == 0 || interval <= 0 || startingPrice <= 0.0) return;

        std::lock_guard<std::mutex> lock(mMutex);
        const int64_t nowSec = nowNs / 1'000'000'000LL;
        const int64_t stepSeconds = static_cast<int64_t>(interval) * 60;
        const int64_t currentBucket = (nowSec / stepSeconds) * stepSeconds;
        const int64_t firstBucket = currentBucket
                                     - static_cast<int64_t>(candleCount) * stepSeconds;

        std::mt19937 generator(seed);
        const double volatility = std::max(startingPrice * 0.00018, 0.00001);
        std::normal_distribution<double> returnDistribution(0.0, volatility);
        std::uniform_real_distribution<double> wickDistribution(0.25, 1.0);
        std::uniform_int_distribution<uint64_t> volumeDistribution(80, 800);

        double price = startingPrice;
        for (size_t index = 0; index < candleCount; ++index) {
            Candle candle;
            candle.symbol = symbol;
            candle.timestamp = firstBucket + static_cast<int64_t>(index) * stepSeconds;
            candle.open = price;
            candle.close = std::max(0.0001, price + returnDistribution(generator));
            const double wick = std::abs(returnDistribution(generator))
                                * wickDistribution(generator);
            candle.high = std::max(candle.open, candle.close) + wick;
            candle.low = std::max(0.0001, std::min(candle.open, candle.close) - wick);
            candle.volume = volumeDistribution(generator);

            if (mCallback) mCallback(interval, candle);
            price = candle.close;
        }
    }

    void CandleAggregator::updateAggregate(Aggregate& agg, int interval, const std::string& symbol, double price, uint64_t qty, int64_t timestampNs) {
        int64_t timestampSec = timestampNs / 1000000000LL;
        int64_t bucketStart = (timestampSec / (interval * 60)) * (interval * 60);

        if (!agg.active || bucketStart > agg.current.timestamp) {
            // Finalize old candle if it exists
            if (agg.active) {
                mRepo.addCandle(interval, agg.current);
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

        // Publish the active candle as soon as it exists and whenever it
        // changes. Waiting for the bucket to close leaves a fresh chart empty
        // for the entire first interval.
        if (mCallback) mCallback(interval, agg.current);
    }

} // namespace ohlc
