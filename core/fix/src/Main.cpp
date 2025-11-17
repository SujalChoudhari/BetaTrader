#include "fix/FixServer.h"
#include "trading_core/TradingCore.h"
#include "logging/Logger.h"
#include "fix/FixRunbookDefinations.h"
#include <asio.hpp>

using namespace trading_core;
using namespace common;

int main() {
    try {
        logging::Logger::Init("fix_server", "logs/fix_server.log", true, true);

        asio::io_context io_context;

        LOG_INFO("Initializing Trading Core...");
        TradingCore tradingCore;
        tradingCore.start();
        LOG_INFO("Trading Core started.");

        fix::FixServer server(io_context, 12345, tradingCore);
        LOG_INFO("FIX Server started on port 12345.");

        // Subscribe to the fully-formed execution reports from the trading core.
        tradingCore.subscribeToExecutions(
            [&](const fix::ExecutionReport& report) {
                // The lambda's only job is to forward the report to the server.
                server.onExecutionReport(report);
            }
        );

        io_context.run();

    } catch (std::exception& e) {
        LOG_CRITICAL(errors::EFIX1, "Exception in main: {}", e.what());
        logging::Logger::Shutdown();
        return 1;
    }

    logging::Logger::Shutdown();
    return 0;
}
