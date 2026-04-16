#include "simulator/StochasticSimulator.h"
#include "logging/Logger.h"
#include <chrono>
#include <cmath>

namespace simulator {

    StochasticSimulator::StochasticSimulator(trading_core::TradingCore& core) 
        : mCore(core), mGen(std::random_device{}()), mDist(0.0, 1.0) 
    {
        // Initialize symbols with realistic starting prices
        std::vector<double> startPrices = {
            1.10850,  // EURUSD (4-digit PIP + pipette)
            154.20,   // USDJPY
            1.2450,   // GBPUSD
            1.3720,   // USDCAD
            83.30,    // USDINR
            90.20,    // EURINR
            104.50,   // GBPINR
            0.6450,   // AUDUSD
            16.50     // USDMXN
        };

        for (int i = 0; i < static_cast<int>(common::Instrument::COUNT); ++i) {
            double price = (i < startPrices.size()) ? startPrices[i] : (100.0 + i);
            mSymbols.push_back({
                price,    // current
                price,    // mean
                0.0005,   // vol (scaled for pips)
                0.1       // mean reversion speed
            });
        }
    }

    StochasticSimulator::~StochasticSimulator() {
        stop();
    }

    void StochasticSimulator::start() {
        start(mNumBots, mIntensity);
    }

    void StochasticSimulator::start(int numBots, double intensity) {
        if (mRunning) return;
        mNumBots = numBots;
        mIntensity = intensity;
        mRunning = true;
        mThread = std::jthread([this]() { run(); });
        LOG_INFO("StochasticSimulator started with {} bots, intensity {}", mNumBots, mIntensity);
    }

    void StochasticSimulator::stop() {
        mRunning = false;
        if (mThread.joinable()) {
            mThread.request_stop();
            mThread.join();
        }
        LOG_INFO("Stochastic Simulator stopped.");
    }

    void StochasticSimulator::run() {
        uint64_t botOrderId = 0;
        auto lastUpdate = std::chrono::steady_clock::now();

        while (mRunning) {
            auto now = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(now - lastUpdate).count();
            lastUpdate = now;

            if (dt <= 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            for (size_t i = 0; i < mSymbols.size(); ++i) {
                common::Instrument inst = static_cast<common::Instrument>(i);
                if (mTargetSymbol != "ALL" && common::to_string(inst) != mTargetSymbol) continue;

                auto& s = mSymbols[i];

                // Ornstein-Uhlenbeck: dx = theta(mu - x)dt + sigma*dW
                double dw = mDist(mGen) * std::sqrt(dt);
                double dx = s.meanReversion * (s.meanPrice - s.currentPrice) * dt + s.volatility * dw;
                s.currentPrice += dx;

                // Probability of a bot placing an order in this tick
                // Intensity * dt * numBots / symbols
                double tradeProb = mIntensity * dt * (mNumBots / (double)mSymbols.size());
                
                std::uniform_real_distribution<double> probDist(0.0, 1.0);
                
                while (tradeProb > 0) {
                    if (probDist(mGen) < std::min(1.0, tradeProb)) {
                        // Place an order
                        bool isBuy = (probDist(mGen) > 0.5);
                        // Offset: within a few pips (0.0001 range)
                        double offset = (mDist(mGen) * 0.0001) + (isBuy ? -0.0002 : 0.0002);
                        double price = s.currentPrice + offset;
                        uint64_t qty = (std::abs(mDist(mGen)) * 10) + 1;

                        uint64_t cid = botOrderId++;
                        common::OrderID coreId = mCore.getOrderIDGenerator()->nextId();

                        std::uniform_int_distribution<int> botDist(1, mNumBots);
                        std::string botId = "BOT_" + std::to_string(botDist(mGen));

                        auto order = std::make_unique<common::Order>(
                            cid, coreId, inst, "SIMULATOR", botId,
                            isBuy ? common::OrderSide::Buy : common::OrderSide::Sell,
                            common::OrderType::Limit, common::TimeInForce::DAY,
                            qty, price, std::chrono::system_clock::now());

                        auto cmd = std::make_unique<trading_core::NewOrder>(
                            "SIMULATOR", std::chrono::system_clock::now(), std::move(order));

                        mCore.submitCommand(std::move(cmd));
                    }
                    tradeProb -= 1.0;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // ~50 ticks per sec
        }
    }

} // namespace simulator
