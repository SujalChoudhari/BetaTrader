#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>

std::atomic<bool> gRunning{true};

void signalHandler(int) {
    gRunning = false;
}

int main(int argc, char** argv) {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    int numAgents = 1;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--agents" && i + 1 < argc) {
            numAgents = std::stoi(argv[++i]);
        }
    }

    std::cout << "BetaTrader Simulator started with " << numAgents << " agents." << std::endl;
    
    // Simulate some activity
    int count = 0;
    while (gRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (++count % 5 == 0) {
            std::cout << "Simulator heart-beat... (Active Agents: " << numAgents << ")" << std::endl;
        }
    }

    std::cout << "Simulator shutting down..." << std::endl;
    return 0;
}
