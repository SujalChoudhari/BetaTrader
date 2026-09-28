#include "App.h"
#include "logging/Logger.h"
#include "client_ui/ClientEventLog.h"
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <unistd.h>

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
        auto core = mExchMgr.getCore();
        if (core != mCurrentCore) {
            if (mLogicInitialized) {
                LOG_INFO("Local exchange state changed, resetting dependent logic...");
                if (mSimulator) mSimulator->stop();
                mSimulator.reset();
                mAggregator.reset();
                mHistoryRepo.reset();
                mLogicInitialized = false;
            }
            
            mCurrentCore = core;
            if (mCurrentCore) {
                initLogic(*mCurrentCore);
            }
        }

        // Render Panels
        mConnPanel.render(mFixSession, mIoCtx, mMarketData);

        const auto selectedSymbol = mMarketData.symbol();
        if (selectedSymbol != mOrderBookSymbol) {
            mOrderBook = std::make_unique<orderbook::OrderBook>(selectedSymbol);
            mOrderBookSymbol = selectedSymbol;
        }

        mExchPanel.render();
        mTradingPanel.render(mFixSession, mMarketData);
        mChartPanel.render(mMarketData);
        mSimPanel.render(mSimulator.get());
        mBookPanel.render(mOrderBook.get(), mMarketData);
        mBlotterPanel.render(mBlotter);

        // Handle session-based message routing
        static std::shared_ptr<fix_client::FixClientSession> lastSession = nullptr;
        if (mFixSession != lastSession) {
            lastSession = mFixSession;
            if (mFixSession) {
                mFixSession->setOrderIntentCallback([this](const fix_client::NewOrderIntent& intent) {
                    try {
                        const auto symbol = common::from_string(intent.symbol);
                        const auto side = intent.side == '1' ? common::OrderSide::Buy
                                                            : common::OrderSide::Sell;
                        const auto orderType = intent.orderType == '1'
                                                       ? common::OrderType::Market
                                                       : common::OrderType::Limit;
                        mBlotter.recordOrderIntent({intent.clientOrderId, symbol, side,
                                                    orderType, intent.price,
                                                    static_cast<common::Quantity>(intent.quantity)});
                    } catch (const std::invalid_argument&) {
                        LOG_WARN("Ignoring order intent with unknown symbol: {}", intent.symbol);
                    }
                });
                mFixSession->setMessageCallback([this](const fix_client::ParsedFixMessage& msg) {
                    if (const auto* report = std::get_if<fix::ExecutionReport>(&msg)) {
                        mBlotter.processExecution(*report);
                    } else if (std::holds_alternative<fix::MarketDataSnapshotFullRefresh>(msg)) {
                        const auto& snapshot = std::get<fix::MarketDataSnapshotFullRefresh>(msg);
                        if (common::to_string(snapshot.symbol) != mMarketData.symbol()) return;
                        mOrderBook->handleSnapshot(snapshot);
                    } else if (std::holds_alternative<fix::MarketDataIncrementalRefresh>(msg)) {
                        const auto& refresh = std::get<fix::MarketDataIncrementalRefresh>(msg);
                        if (common::to_string(refresh.symbol) != mMarketData.symbol()) return;
                        mOrderBook->handleIncremental(refresh);
                    }
                });
            }
        }

        mUI.endFrame();
    }

    LOG_INFO("Shutting down BetaTrader Client...");
    return 0;
}

int App::runSmoke() {
    logging::Logger::Init("client_working_client_smoke",
                          "logs/client_working_client_smoke.log", true, false);

    const short port = static_cast<short>(19000 + (getpid() % 1000));
    const auto sequenceDirectory = std::filesystem::temp_directory_path()
                                   / ("betatrader-client-smoke-"
                                      + std::to_string(getpid()));
    std::filesystem::remove_all(sequenceDirectory);

    std::mutex stateMutex;
    std::condition_variable stateChanged;
    bool active = false;
    bool disconnected = false;
    std::string clientOrderId;
    client_ui::ClientEventLog eventLog;
    client_ui::MarketDataState marketData;
    orderbook::OrderBook orderBook("EURUSD");
    client_blotter::BlotterModel blotter;

    auto waitFor = [&](const auto& predicate, const std::chrono::seconds timeout) {
        std::unique_lock<std::mutex> lock(stateMutex);
        return stateChanged.wait_for(lock, timeout, predicate);
    };

    auto finish = [&](bool success, const std::string& reason = std::string{}) {
        if (mFixSession) mFixSession->disconnect();
        mExchMgr.stop();
        mFixSession.reset();
        std::error_code error;
        std::filesystem::remove_all(sequenceDirectory, error);
        if (!success) {
            LOG_ERROR("Working-client smoke failed: {}", reason);
            return 1;
        }
        LOG_INFO("Working-client smoke passed: connected, subscribed, routed order and market data, and disconnected cleanly.");
        return 0;
    };

    try {
        if (!mExchMgr.start(port) || !mExchMgr.getServer()) {
            return finish(false, "local exchange did not start");
        }
        // ExchangeManager seeds CLIENT1; load it synchronously as well so the
        // smoke flow cannot race the asynchronous repository callback.
        mExchMgr.getServer()->getManager().loadConfig({"CLIENT1"});

        mOrderBook = std::make_unique<orderbook::OrderBook>("EURUSD");
        mFixSession = std::make_shared<fix_client::FixClientSession>(
                mIoCtx, "CLIENT1", "BETA_EXCHANGE", sequenceDirectory.string());
        mFixSession->setEventCallback([&eventLog](const fix_client::FixClientEvent& event) {
            eventLog.append(event);
        });
        mFixSession->setStateChangeCallback([&](fix_client::FixClientState state) {
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                active = state == fix_client::FixClientState::Active;
                disconnected = state == fix_client::FixClientState::Disconnected;
            }
            stateChanged.notify_all();
        });
        mFixSession->setOrderIntentCallback([&](const fix_client::NewOrderIntent& intent) {
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                clientOrderId = intent.clientOrderId;
            }
            blotter.recordOrderIntent({intent.clientOrderId,
                                       common::from_string(intent.symbol),
                                       intent.side == '1' ? common::OrderSide::Buy
                                                          : common::OrderSide::Sell,
                                       intent.orderType == '1' ? common::OrderType::Market
                                                               : common::OrderType::Limit,
                                       intent.price,
                                       static_cast<common::Quantity>(intent.quantity)});
            stateChanged.notify_all();
        });
        mFixSession->setMessageCallback([&](const fix_client::ParsedFixMessage& message) {
            if (const auto* report = std::get_if<fix::ExecutionReport>(&message)) {
                blotter.processExecution(*report);
            } else if (const auto* snapshot =
                               std::get_if<fix::MarketDataSnapshotFullRefresh>(&message)) {
                if (common::to_string(snapshot->symbol) == marketData.symbol()) {
                    orderBook.handleSnapshot(*snapshot);
                }
            } else if (const auto* refresh =
                               std::get_if<fix::MarketDataIncrementalRefresh>(&message)) {
                if (common::to_string(refresh->symbol) == marketData.symbol()) {
                    orderBook.handleIncremental(*refresh);
                }
            }
            stateChanged.notify_all();
        });

        mFixSession->connect("127.0.0.1", port);
        if (!waitFor([&] {
                return mFixSession->getState() == fix_client::FixClientState::Connected;
            }, std::chrono::seconds(3))) {
            return finish(false, "client did not reach TCP connected state");
        }

        mFixSession->sendLogon(5, true);
        if (!waitFor([&] { return active; }, std::chrono::seconds(3))) {
            return finish(false, "client did not reach active state");
        }
        marketData.markSessionReady();

        mFixSession->sendMarketDataRequest(marketData.symbol(), '1');
        marketData.markSubscribed();

        if (!waitFor([&] {
                const auto events = eventLog.snapshot();
                return std::any_of(
                        events.begin(), events.end(), [](const auto& event) {
                            return event.type == fix_client::FixClientEventType::MarketData
                                   && event.message == "Snapshot EURUSD entries=0";
                        });
            }, std::chrono::seconds(3))) {
            return finish(false, "market-data subscription did not produce its initial snapshot");
        }

        const auto sessionId = static_cast<fix::CompID>(
                mExchMgr.getServer()->getSessions().begin()->first);
        fix::MarketDataSnapshotFullRefresh snapshot;
        snapshot.targetSessionID = sessionId;
        snapshot.mdReqID = "client-smoke";
        snapshot.symbol = common::Instrument::EURUSD;
        snapshot.entries = {{fix::MDEntryType::Bid, 1.2345, 100, {}, 0},
                            {fix::MDEntryType::Offer, 1.2347, 120, {}, 0}};
        mExchMgr.getServer()->onMarketDataSnapshotFullRefresh(snapshot);

        if (!waitFor([&] {
                const auto snapshotState = orderBook.getUISnapshot();
                return snapshotState.bids.size() == 1U && snapshotState.asks.size() == 1U;
            }, std::chrono::seconds(3))) {
            return finish(false, "market-data snapshot did not reach the client order book");
        }

        fix::MarketDataIncrementalRefresh refresh;
        refresh.targetSessionID = sessionId;
        refresh.mdReqID = "client-smoke";
        refresh.symbol = common::Instrument::EURUSD;
        refresh.entries = {{fix::MDUpdateAction::New, fix::MDEntryType::Bid,
                             1.2346, 140, {}, 0}};
        mExchMgr.getServer()->onMarketDataIncrementalRefresh(refresh);
        if (!waitFor([&] {
                const auto snapshotState = orderBook.getUISnapshot();
                return snapshotState.bids.size() == 2U;
            }, std::chrono::seconds(3))) {
            return finish(false, "market-data incremental did not update the client order book");
        }

        mFixSession->sendNewOrder("EURUSD", '1', 1.2345, 100, '2', '0');
        if (!waitFor([&] { return !clientOrderId.empty(); }, std::chrono::seconds(3))) {
            return finish(false, "order entry did not produce a client order intent");
        }

        if (!waitFor([&] {
                const auto orders = blotter.activeOrders();
                const auto history = blotter.history();
                const auto events = eventLog.snapshot();
                const bool sawExecution = std::any_of(
                        events.begin(), events.end(), [](const auto& event) {
                            return event.type == fix_client::FixClientEventType::ExecutionReport;
                        });
                const bool sawMarketData = std::any_of(
                        events.begin(), events.end(), [](const auto& event) {
                            return event.type == fix_client::FixClientEventType::MarketData;
                        });
                return orders.size() == 1U && orders.front().clientOrderId == clientOrderId
                       && orders.front().remainingQuantity == 100U && history.size() == 1U
                       && sawExecution && sawMarketData;
            }, std::chrono::seconds(3))) {
            return finish(false, "execution feedback did not reach the open-orders model");
        }

        const auto history = blotter.history();
        const auto events = eventLog.snapshot();
        const bool sawExecution = std::any_of(
                events.begin(), events.end(), [](const auto& event) {
                    return event.type == fix_client::FixClientEventType::ExecutionReport;
                });
        const bool sawMarketData = std::any_of(
                events.begin(), events.end(), [](const auto& event) {
                    return event.type == fix_client::FixClientEventType::MarketData;
                });
        if (history.empty() || !sawExecution || !sawMarketData
            || marketData.status() != client_ui::MarketDataSubscriptionStatus::Subscribed) {
            return finish(false, "session log or client models did not retain the routed events");
        }

        mFixSession->sendLogout("working-client smoke complete");
        if (!waitFor([&] { return disconnected; }, std::chrono::seconds(3))) {
            return finish(false, "client did not return to disconnected state");
        }
    } catch (const std::exception& error) {
        return finish(false, error.what());
    }

    return finish(true);
}

void App::initLogic(trading_core::TradingCore& core) {
    LOG_INFO("Initializing local exchange logic components...");
    mHistoryRepo = std::make_unique<data::MarketHistoryRepository>(core.getDatabaseWorker());
    mAggregator = std::make_unique<ohlc::CandleAggregator>(*mHistoryRepo);
    mSimulator = std::make_unique<simulator::StochasticSimulator>(core);

    mAggregator->setCandleCallback([this](int interval, const ohlc::Candle& candle) {
        mChartPanel.onCandleUpdate(interval, candle, mMarketData);
    });

    const auto selectedSymbol = mMarketData.symbol();
    double startingPrice = 1.1085;
    try {
        switch (common::from_string(selectedSymbol)) {
            case common::Instrument::EURUSD: startingPrice = 1.1085; break;
            case common::Instrument::USDJPY: startingPrice = 154.20; break;
            case common::Instrument::GBPUSD: startingPrice = 1.2450; break;
            case common::Instrument::USDCAD: startingPrice = 1.3720; break;
            case common::Instrument::USDINR: startingPrice = 83.30; break;
            case common::Instrument::EURINR: startingPrice = 90.20; break;
            case common::Instrument::GBPINR: startingPrice = 104.50; break;
            case common::Instrument::AUDUSD: startingPrice = 0.6450; break;
            case common::Instrument::USDMXN: startingPrice = 16.50; break;
            case common::Instrument::COUNT: break;
        }
    } catch (const std::invalid_argument&) {
        LOG_WARN("Using default chart seed price for unknown symbol {}", selectedSymbol);
    }
    const auto nowNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    mAggregator->seedHistoricalData(selectedSymbol, 1, 60, nowNs, startingPrice, 0xB37A01U);
    mAggregator->seedHistoricalData(selectedSymbol, 5, 36, nowNs, startingPrice, 0xB37A05U);

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
