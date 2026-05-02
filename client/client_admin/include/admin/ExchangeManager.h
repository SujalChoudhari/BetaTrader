#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

namespace admin {

/**
 * @class ExchangeManager
 * @brief Handles starting and stopping the exchange server and simulator.
 */
class ExchangeManager {
public:
    ExchangeManager();
    ~ExchangeManager();

    /**
     * @brief Starts the exchange server process.
     */
    bool startExchange(const std::string& binaryPath);

    /**
     * @brief Stops the exchange server process.
     */
    void stopExchange();

    /**
     * @brief Starts the simulator process with a specific number of agents.
     */
    bool startSimulator(const std::string& binaryPath, int numAgents);

    /**
     * @brief Stops the simulator process.
     */
    void stopSimulator();

    bool isExchangeRunning() const { return mExchangeRunning; }
    bool isSimulatorRunning() const { return mSimulatorRunning; }

private:
    std::atomic<bool> mExchangeRunning{false};
    std::atomic<bool> mSimulatorRunning{false};
    
    pid_t mExchangePid{-1};
    pid_t mSimulatorPid{-1};
};

} // namespace admin
