#pragma once

#include "exchange_app/TradingCore.h"
#include "common/Instrument.h"
#include <atomic>
#include <random>
#include <thread>
#include <vector>

namespace simulator {

    /**
     * @class StochasticSimulator
     * @brief Simulates thousands of bots trading realistically using a stochastic process.
     * 
     * Injects orders directly into TradingCore to bypass network overhead.
     */
    class StochasticSimulator {
    public:
        explicit StochasticSimulator(trading_core::TradingCore& core);
        ~StochasticSimulator();

        void start();
        void start(int numBots, double intensity);
        void stop();

        bool isRunning() const { return mRunning; }
        size_t getBotCount() const { return static_cast<size_t>(mNumBots); }
        void setBotCount(size_t count) { mNumBots = static_cast<int>(count); }
        double getIntensity() const { return mIntensity; }
        void setIntensity(double intensity) { mIntensity = intensity; }
        void setSymbol(const std::string& symbol) { mTargetSymbol = symbol; }

    private:
        void run();
        
        trading_core::TradingCore& mCore;
        std::atomic<bool> mRunning{false};
        std::jthread mThread;
        
        int mNumBots = 0;
        double mIntensity = 1.0;
        std::string mTargetSymbol = "ALL";

        // Per-symbol stochastic state
        struct SymbolState {
            double currentPrice;
            double meanPrice;
            double volatility;
            double meanReversion;
        };
        std::vector<SymbolState> mSymbols;

        std::mt19937 mGen;
        std::normal_distribution<double> mDist;
    };

} // namespace simulator
