#include "fix/FixRunbookDefinations.h"
#include "fix/FixServer.h"
#include "logging/Logger.h"
#include "trading_core/TradingCore.h"
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

        // TODO: Make port configurable, e.g., via command line arguments or a config file.
        fix::FixServer server(io_context, 8088, tradingCore);
        LOG_INFO("FIX Server started on port 8088.");

        tradingCore.subscribeToExecutions(
            [&](const fix::ExecutionReport& report) {
                server.onExecutionReport(report);
            }
        );

        tradingCore.subscribeToMarketDataSnapshots(
            [&](const fix::MarketDataSnapshotFullRefresh& snapshot) {
                server.onMarketDataSnapshotFullRefresh(snapshot);
            }
        );

        tradingCore.subscribeToMarketDataIncrements(
            [&](const fix::MarketDataIncrementalRefresh& refresh) {
                server.onMarketDataIncrementalRefresh(refresh);
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
