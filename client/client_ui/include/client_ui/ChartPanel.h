#pragma once

#include "ohlc/CandleAggregator.h"
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

        void render();

        // Feed data into the chart (called by CandleAggregator callback)
        void onCandleUpdate(int interval, const ohlc::Candle& candle);

    private:
        char mSymbol[16] = "EURUSD";
        int mInterval = 1;
        
        mutable std::mutex mMutex;
        // Current candles being displayed
        std::vector<ohlc::Candle> mCandles;

        void drawCandleChart();
    };

} // namespace client_ui
