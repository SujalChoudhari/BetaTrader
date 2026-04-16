#pragma once

#include <exchange_app/TradingCore.h>
#include <fix/FixServer.h>
#include <data/SequenceRepository.h>
#include <asio.hpp>

#include <memory>
#include <atomic>

namespace admin {

/**
 * @class ExchangeManager
 * @brief Manages the lifecycle of a local Exchange instance (TradingCore + FixServer).
 */
class ExchangeManager {
public:
    explicit ExchangeManager(asio::io_context& ioCtx);
    ~ExchangeManager();

    /**
     * @brief Starts the exchange services.
     * @param port TCP port for the FIX server.
     * @return true if successfully started.
     */
    bool start(short port = 8088);

    /**
     * @brief Stops all exchange services.
     */
    void stop();

    bool isRunning() const { return mIsRunning; }
    
    trading_core::TradingCore* getCore() { return mTradingCore.get(); }
    fix::FixServer* getServer() { return mFixServer.get(); }

private:
    asio::io_context& mIoCtx;
    std::unique_ptr<trading_core::TradingCore> mTradingCore;
    std::unique_ptr<fix::FixServer> mFixServer;
    std::unique_ptr<data::SequenceRepository> mSeqRepo;
    std::atomic<bool> mIsRunning{false};

};

} // namespace admin
