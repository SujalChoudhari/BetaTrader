#include "admin/ExchangeManager.h"
#include "logging/Logger.h"
#include <filesystem>

namespace admin {

ExchangeManager::ExchangeManager(asio::io_context& ioCtx) : mIoCtx(ioCtx) {}

ExchangeManager::~ExchangeManager() {
    stop();
}

bool ExchangeManager::start(short port) {
    if (mIsRunning) return true;

    try {
        // Local exchange uses in-memory DB, so stale client seq files
        // will cause fatal sequence mismatches. Clean them.
        if (std::filesystem::exists("seq_store")) {
            std::filesystem::remove_all("seq_store");
            LOG_INFO("Admin: Cleaned stale client sequence store for fresh local exchange.");
        }

        LOG_INFO("Admin: Initializing local TradingCore...");
        mTradingCore = std::make_unique<trading_core::TradingCore>();
        
        // Seed default authorized client for local testing
        if (auto* authRepo = mTradingCore->getAuthRepository()) {
            authRepo->initDatabase();
            authRepo->insertNewClient("CLIENT1", true);
            LOG_INFO("Admin: Registered default client 'CLIENT1' in local exchange.");
        }

        mTradingCore->start();

        // Initialize Sequence Persistence
        mSeqRepo = std::make_unique<data::SequenceRepository>(mTradingCore->getDatabaseWorker());
        mSeqRepo->initDatabase();

        LOG_INFO("Admin: Starting local FIX Server on port {}...", port);
        mFixServer = std::make_unique<fix::FixServer>(mIoCtx, port, *mTradingCore, mSeqRepo.get());


        mIsRunning = true;

        LOG_INFO("Admin: Local Exchange is ONLINE.");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Admin: Failed to start local exchange: {}", e.what());
        stop();
        return false;
    }
}

void ExchangeManager::stop() {
    if (!mIsRunning) return;

    LOG_INFO("Admin: Stopping local Exchange services...");
    if (mFixServer) {
        mFixServer->stop();
        mFixServer.reset();
    }
    mSeqRepo.reset();
    if (mTradingCore) {

        mTradingCore->stop();
        mTradingCore.reset();
    }

    mIsRunning = false;
    LOG_INFO("Admin: Local Exchange is OFFLINE.");
}

} // namespace admin
