#include "App.h"
#include "logging/Logger.h"
#include <iostream>

namespace client_app {

App::App() : mWork(asio::make_work_guard(mIoCtx)), mExchMgr(mIoCtx), mExchPanel(mExchMgr) {
    mNetworkThread = std::thread([this]() {
        try {
            mIoCtx.run();
        } catch (const std::exception& e) {
            LOG_CRITICAL("Network thread exception: {}", e.what());
        }
    });
}

App::~App() {
    if (mSimulator) mSimulator->stop();
    mWork.reset();
    mIoCtx.stop();

    if (mNetworkThread.joinable()) {
        mNetworkThread.join();
    }
    mUI.stop();
}

int App::run() {
    logging::Logger::Init("client_app", "logs/client_app.log", true, true, spdlog::level::trace, 8192, 1, 1024 * 1024 * 300, 5);
    LOG_INFO("Starting BetaTrader Client...");

    if (!mUI.start("BetaTrader Client Dashboard", 1440, 900)) {
        return 1;
    }

    // Initialize base UI-driven components
    mOrderBook = std::make_unique<orderbook::OrderBook>("EURUSD");
    
    while (!mUI.shouldClose()) {
        mUI.beginFrame();
        mUI.renderDockspace();

        // Check for local exchange core to initialize advanced logic
        auto core = mExchMgr.getServer() ? &mExchMgr.getServer()->getTradingCore() : nullptr;
        if (core && !mLogicInitialized) {
            initLogic(*core);
        }

        // Render Panels
        mConnPanel.render(mFixSession, mIoCtx);
        mExchPanel.render();
        mTradingPanel.render(mFixSession);
        mChartPanel.render();
        mSimPanel.render(mSimulator.get());
        mBookPanel.render(mOrderBook.get());

        // Handle session-based message routing
        static std::shared_ptr<fix_client::FixClientSession> lastSession = nullptr;
        if (mFixSession != lastSession) {
            lastSession = mFixSession;
            if (mFixSession) {
                mFixSession->setMessageCallback([this](const fix_client::ParsedFixMessage& msg) {
                    if (std::holds_alternative<fix::MarketDataSnapshotFullRefresh>(msg)) {
                        mOrderBook->handleSnapshot(std::get<fix::MarketDataSnapshotFullRefresh>(msg));
                    } else if (std::holds_alternative<fix::MarketDataIncrementalRefresh>(msg)) {
                        mOrderBook->handleIncremental(std::get<fix::MarketDataIncrementalRefresh>(msg));
                    }
                });
            }
        }

        mUI.endFrame();
    }

    LOG_INFO("Shutting down BetaTrader Client...");
    return 0;
}

void App::initLogic(trading_core::TradingCore& core) {
    LOG_INFO("Initializing local exchange logic components...");
    mHistoryRepo = std::make_unique<data::MarketHistoryRepository>(core.getDatabaseWorker());
    mAggregator = std::make_unique<ohlc::CandleAggregator>(*mHistoryRepo);
    mSimulator = std::make_unique<simulator::StochasticSimulator>(core);

    mAggregator->setCandleCallback([this](int interval, const ohlc::Candle& candle) {
        mChartPanel.onCandleUpdate(interval, candle);
    });

    // Hook aggregator to TradingCore trade events using General subscriber
    core.getMarketDataPublisher().addGeneralIncrementalSubscriber([this](const fix::MarketDataIncrementalRefresh& refresh) {
        if (!mAggregator) return;
        for (const auto& entry : refresh.entries) {
            if (entry.updateAction == fix::MDUpdateAction::Delete) continue;
            mAggregator->onTrade(common::to_string(refresh.symbol), entry.price, entry.size, 
                std::chrono::system_clock::now().time_since_epoch().count());
        }
    });

    mLogicInitialized = true;
}

} // namespace client_app
