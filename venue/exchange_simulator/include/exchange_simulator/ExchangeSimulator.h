#pragma once

#include "trading_core/TradingCore.h"
#include "common/Order.h"
#include "common/Types.h"
#include "trading_core/NewOrder.h"
#include "trading_core/ModifyOrder.h"
#include "trading_core/CancelOrder.h"

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>
#include <map>

namespace exchange_simulator {

class ExchangeSimulator {
public:
    ExchangeSimulator();
    ~ExchangeSimulator();

    void run(int numOrdersToSimulate);

private:
    std::unique_ptr<trading_core::TradingCore> mTradingCore;

    // For capturing cout output from ExecutionPublisher
    std::stringstream mCoutBuffer;
    std::streambuf* mOldCoutBuffer;

    // Simple market data for simulation
    std::map<common::Instrument, double> mCurrentPrices;
    std::random_device mRandomDevice;
    std::mt19937 mRandomGenerator;

    void setupTradingCore();
    void generateAndSubmitOrder(common::OrderID orderId);
    void processExecutionOutput();
    void initializeMarketData();
};

} // namespace exchange_simulator
