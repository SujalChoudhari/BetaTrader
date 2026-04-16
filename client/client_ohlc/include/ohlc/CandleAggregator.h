#pragma once

#include "common/Types.h"
#include "data/MarketHistoryRepository.h"
#include <map>
#include <mutex>
#include <vector>
#include <functional>

namespace ohlc {

    using namespace data; // For Candle struct

    /**
     * @class CandleAggregator
     * @brief Aggregates trade updates into OHLC candles and persists them.
     */
    class CandleAggregator {
    public:
        using CandleCallback = std::function<void(int interval, const Candle&)>;

        CandleAggregator(MarketHistoryRepository& repo);
        ~CandleAggregator() = default;

        // Process a new trade update
        void onTrade(const std::string& symbol, double price, uint64_t qty, int64_t timestampNs);

        void setCandleCallback(CandleCallback cb) { mCallback = std::move(cb); }

    private:
        MarketHistoryRepository& mRepo;
        CandleCallback mCallback;

        struct Aggregate {
            Candle current;
            bool active = false;
        };

        // symbol -> interval -> Aggregate
        std::map<std::string, std::map<int, Aggregate>> mAggregates;
        std::mutex mMutex;

        void updateAggregate(Aggregate& agg, int interval, const std::string& symbol, double price, uint64_t qty, int64_t timestampNs);
    };

} // namespace ohlc
