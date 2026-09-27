#pragma once

#include "ohlc/CandleAggregator.h"
#include "client_ui/MarketDataState.h"
#include <string>
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
        // Current candles being displayed
        std::vector<ohlc::Candle> mCandles;
        std::string mDisplayedSymbol = "EURUSD";

        void drawCandleChart();
    };

} // namespace client_ui
