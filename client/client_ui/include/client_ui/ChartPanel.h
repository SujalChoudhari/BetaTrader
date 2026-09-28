#pragma once

#include "ohlc/CandleAggregator.h"
#include "client_ui/MarketDataState.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

namespace client_ui {

    /**
     * @class ChartPanel
     * @brief Renders OHLC candlestick charts using ImPlot.
     */
    class ChartPanel {
    public:
        ChartPanel();
        ~ChartPanel() = default;

        void render(const MarketDataState& marketData);

        // Feed data into the chart (called by CandleAggregator callback)
        void onCandleUpdate(int interval,
                            const ohlc::Candle& candle,
                            const MarketDataState& marketData);

    private:
        int mInterval = 1;
        
        mutable std::mutex mMutex;
        // Cached history for each supported interval, including live updates.
        std::map<int, std::vector<ohlc::Candle>> mCandlesByInterval;
        std::string mDisplayedSymbol = "EURUSD";

        void drawCandleChart();
    };

} // namespace client_ui
