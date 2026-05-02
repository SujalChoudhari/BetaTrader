#include "fix_client/FixClientSession.h"
#include "logging/Logger.h"
#include "common/Instrument.h"
#include <asio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>
#include <random>
#include <map>
#include <iomanip>
#include <sstream>

std::atomic<bool> gRunning{true};

void signalHandler(int) {
    gRunning = false;
}

std::string formatAgentId(const std::string& prefix, int id) {
    std::ostringstream oss;
    oss << prefix << std::setfill('0') << std::setw(4) << id;
    return oss.str();
}

int main(int argc, char** argv) {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    int numAgents = 50;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--agents" && i + 1 < argc) {
            numAgents = std::stoi(argv[i+1]);
        }
    }

    logging::Logger::Init("simulator", "logs/simulator.log", true, true);

    asio::io_context ioContext;
    auto work = asio::make_work_guard(ioContext);

    std::vector<std::shared_ptr<fix_client::FixClientSession>> groupA;
    std::vector<std::shared_ptr<fix_client::FixClientSession>> groupB;

    int half = numAgents / 2;
    for (int i = 0; i < half; ++i) {
        std::string idA = formatAgentId("SIM_A_", i);
        std::string idB = formatAgentId("SIM_B_", i);
        
        auto sa = std::make_shared<fix_client::FixClientSession>(ioContext, idA, "BETA_EXCHANGE", "seq_store/" + idA + ".seq");
        auto sb = std::make_shared<fix_client::FixClientSession>(ioContext, idB, "BETA_EXCHANGE", "seq_store/" + idB + ".seq");
        
        groupA.push_back(sa);
        groupB.push_back(sb);
    }

    std::vector<std::thread> threadPool;
    for (int i = 0; i < 4; ++i) {
        threadPool.emplace_back([&ioContext]() {
            try { ioContext.run(); } catch (...) {}
        });
    }

    std::cout << "Simulator: Connecting " << numAgents << " agents..." << std::endl;
    for (int i = 0; i < half; ++i) {
        groupA[i]->connect("127.0.0.1", 8088);
        groupB[i]->connect("127.0.0.1", 8088);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "Simulator: Reseting sequence for all agents..." << std::endl;
    for (int i = 0; i < half; ++i) {
        groupA[i]->sendLogon(30, true); 
        groupB[i]->sendLogon(30, true);
    }

    std::map<std::string, double> basePrices = {
        {"EURUSD", 1.0850}, {"USDJPY", 151.20}, {"GBPUSD", 1.2640},
        {"USDCAD", 1.3550}, {"USDINR", 83.30}, {"EURINR", 90.50},
        {"GBPINR", 105.20}, {"AUDUSD", 0.6540}, {"USDMXN", 16.70}
    };

    std::mt19937 rng(std::random_device{}());
    
    struct MarketState {
        double trend = 0.0; // -1.0 to 1.0
        int trendDuration = 0;
    };
    std::map<std::string, MarketState> marketStates;

    std::cout << "All " << numAgents << " agents active. Starting trend-following simulation loop..." << std::endl;

    while (gRunning) {
        // 1. Move prices with momentum and trends
        for (auto& [symbol, base] : basePrices) {
            auto& state = marketStates[symbol];
            
            // Randomly flip or strengthen trend
            if (state.trendDuration <= 0 || (rng() % 100 < 5)) {
                state.trend = (static_cast<double>(static_cast<int>(rng() % 200) - 100) / 100.0);
                state.trendDuration = (rng() % 50) + 10; // Trend lasts 10-60 cycles
            }
            state.trendDuration--;

            // Momentum: 80% chance to follow trend, 20% noise
            double bias = (rng() % 100 < 80) ? state.trend : 0.0;
            int randomShift = static_cast<int>(rng() % 100) - 50;
            double walk = (static_cast<double>(randomShift) / 80000.0 + (bias / 10000.0)) * base;
            base += walk;
        }

        for (int i = 0; i < half; ++i) {
            auto& sa = groupA[i];
            auto& sb = groupB[i];

            if (sa->getState() != fix_client::FixClientState::Active || 
                sb->getState() != fix_client::FixClientState::Active) continue;

            std::string symbol(common::symbol_names[rng() % common::symbol_names.size()]);
            double base = basePrices[symbol];
            
            // Tighter institutional spreads (0.5 to 1.5 pips)
            std::uniform_real_distribution<double> spreadDist(0.00005 * base, 0.00015 * base);
            double currentSpread = spreadDist(rng);
            double bid = base - (currentSpread / 2.0);
            double ask = base + (currentSpread / 2.0);
            int qty = ((rng() % 10 + 1) * 1000);

            if (rng() % 10 < 8) {
                sa->sendNewOrder(symbol, '1', bid, qty, '2'); 
                sb->sendNewOrder(symbol, '2', ask, qty, '2'); 
            } else {
                sa->sendNewOrder(symbol, '1', ask, qty, '1'); 
                sb->sendNewOrder(symbol, '2', bid, qty, '1'); 
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    gRunning = false;
    ioContext.stop();
    for (auto& t : threadPool) if (t.joinable()) t.join();

    std::cout << "Simulator shutting down..." << std::endl;
    return 0;
}
