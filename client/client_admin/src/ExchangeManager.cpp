#include <admin/ExchangeManager.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <cerrno>

namespace admin {

ExchangeManager::ExchangeManager() {}

ExchangeManager::~ExchangeManager() {
    stopExchange();
    stopSimulator();
}

std::string findBinary(const std::string& name) {
    namespace fs = std::filesystem;
    
    // 1. Try current directory
    if (fs::exists("./" + name)) return "./" + name;
    
    // 2. Try relative paths in build tree
    // If we are in build/client/client_app, the binary might be in ../../exchange/exchange_fix/
    std::vector<std::string> searchPaths = {
        "../../exchange/exchange_fix/",
        "../../../exchange/exchange_fix/",
        "../bin/",
        "../../bin/",
        "./exchange/exchange_fix/"
    };
    
    for (const auto& path : searchPaths) {
        if (fs::exists(path + name)) return path + name;
    }
    
    // 3. Fallback: Search the entire project root build directory if possible
    // This is expensive but better than failing
    try {
        // Assume we are somewhere inside 'build'
        fs::path current = fs::current_path();
        while (current.has_parent_path() && current.filename() != "BetaTrader") {
            current = current.parent_path();
        }
        
        if (current.filename() == "BetaTrader" && fs::exists(current / "build")) {
            for (const auto& entry : fs::recursive_directory_iterator(current / "build")) {
                if (entry.is_regular_file() && entry.path().filename() == name) {
                    return entry.path().string();
                }
            }
        }
    } catch (...) {}

    return name; // Return original if not found, let execvp try PATH
}

bool ExchangeManager::startExchange(const std::string& binaryPath) {
    if (mExchangeRunning) return true;

    std::string actualPath = findBinary("exchange_app");
    std::cout << "[Admin] Resolved Exchange Path: " << actualPath << std::endl;

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char* args[] = { (char*)actualPath.c_str(), nullptr };
        if (execvp(args[0], args) == -1) {
            std::cerr << "[Admin] Failed to start Exchange: " << actualPath << " (Error: " << errno << ")" << std::endl;
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        mExchangePid = pid;
        mExchangeRunning = true;
        return true;
    } else {
        std::cerr << "[Admin] Failed to fork for Exchange." << std::endl;
        return false;
    }
    return false;
}

void ExchangeManager::stopExchange() {
    if (!mExchangeRunning || mExchangePid == -1) return;

    std::cout << "[Admin] Stopping Exchange (PID: " << mExchangePid << ")..." << std::endl;
    kill(mExchangePid, SIGTERM);
    
    int status;
    waitpid(mExchangePid, &status, WNOHANG);
    
    mExchangePid = -1;
    mExchangeRunning = false;
}

bool ExchangeManager::startSimulator(const std::string& binaryPath, int numAgents) {
    if (mSimulatorRunning) return true;

    std::string actualPath = findBinary("client_simulator");
    std::cout << "[Admin] Resolved Simulator Path: " << actualPath << std::endl;

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::string agentsStr = std::to_string(numAgents);
        char* args[] = { (char*)actualPath.c_str(), (char*)"--agents", (char*)agentsStr.c_str(), nullptr };
        if (execvp(args[0], args) == -1) {
            std::cerr << "[Admin] Failed to start Simulator: " << actualPath << " (Error: " << errno << ")" << std::endl;
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        mSimulatorPid = pid;
        mSimulatorRunning = true;
        return true;
    } else {
        std::cerr << "[Admin] Failed to fork for Simulator." << std::endl;
        return false;
    }
    return false;
}

void ExchangeManager::stopSimulator() {
    if (!mSimulatorRunning || mSimulatorPid == -1) return;

    std::cout << "[Admin] Stopping Simulator (PID: " << mSimulatorPid << ")..." << std::endl;
    kill(mSimulatorPid, SIGTERM);
    
    int status;
    waitpid(mSimulatorPid, &status, WNOHANG);
    
    mSimulatorPid = -1;
    mSimulatorRunning = false;
}

} // namespace admin
