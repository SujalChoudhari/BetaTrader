#include "fix/FixRunbookDefinations.h"
#include "fix/FixServer.h"
#include "logging/Logger.h"
#include "trading_core/TradingCore.h"
#include "data/SequenceRepository.h"
#include <asio.hpp>
#include <iostream>

using namespace trading_core;
using namespace common;

int main(int argc, char** argv) {
    short port = 8088; // Default port

    if (argc >= 2) {
        try {
            port = static_cast<short>(std::stoi(argv[1]));
        } catch (const std::exception& e) {
            std::cerr << "Invalid port provided: " << argv[1] << ". Using default 8088." << std::endl;
        }
    }

    try {
        logging::Logger::Init("fix_server", "logs/fix_server.log", true, true);

        asio::io_context io_context;

        LOG_DEBUG("Initializing Trading Core...");
        TradingCore tradingCore;
        tradingCore.start();
        LOG_INFO("Trading Core started.");

        std::shared_ptr<data::SequenceRepository> seqRepo = nullptr;
        if (auto* dbWorker = tradingCore.getDatabaseWorker()) {
            seqRepo = std::make_shared<data::SequenceRepository>(dbWorker);
            seqRepo->initDatabase();
        }

        fix::FixServer server(io_context, port, tradingCore, seqRepo.get());
        LOG_INFO("FIX Server started on port {}.", port);

        tradingCore.subscribeToExecutions(
            [&](const fix::ExecutionReport& report) {
                server.onExecutionReport(report);
            }
        );

        // Subscribe to market data publisher
        tradingCore.getMarketDataPublisher().subscribeToSnapshots(
            [&](const fix::MarketDataSnapshotFullRefresh& snapshot) {
                server.onMarketDataSnapshotFullRefresh(snapshot);
            }
        );

        tradingCore.getMarketDataPublisher().subscribeToIncrementals(
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
