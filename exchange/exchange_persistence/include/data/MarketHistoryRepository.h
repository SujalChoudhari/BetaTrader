#pragma once

#include "common/Instrument.h"
#include "data/DatabaseWorker.h"
#include <string>
#include <vector>

namespace data {

    struct Candle {
        std::string symbol;
        int64_t timestamp; // Unix timestamp for the start of the bucket
        double open;
        double high;
        double low;
        double close;
        uint64_t volume;
    };

    /**
     * @class MarketHistoryRepository
     * @brief Persists OHLC candle data to the database.
     */
    class MarketHistoryRepository {
    public:
        explicit MarketHistoryRepository(DatabaseWorker* dbWorker);
        virtual ~MarketHistoryRepository() = default;

        virtual void initDatabase();
        
        // interval is in minutes (e.g. 1, 5, 60)
        virtual void addCandle(int interval, const Candle& candle);
        
        virtual std::vector<Candle> getHistory(const std::string& symbol, int interval, int limit = 100);

    private:
        DatabaseWorker* mDb;
    };

} // namespace data
