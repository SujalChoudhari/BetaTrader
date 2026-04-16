#pragma once

#include "client_ui/UIManager.h"
#include "client_ui/ConnectionPanel.h"
#include "admin/ExchangeManager.h"
#include "admin/ExchangePanel.h"
#include "client_ui/TradingPanel.h"
#include "client_ui/ChartPanel.h"
#include "client_ui/SimulatorPanel.h"
#include "client_ui/OrderBookPanel.h"
#include "orderbook/OrderBook.h"
#include "ohlc/CandleAggregator.h"
#include "simulator/StochasticSimulator.h"
#include "data/MarketHistoryRepository.h"
#include <asio.hpp>

#include <thread>
#include <memory>

namespace client_app {

/**
 * @class App
 * @brief Coordinates the lifecycle of the network and UI threads.
 */
class App {
public:
    App();
    ~App();

    /**
     * @brief Entry point that runs the application until closure.
     */
    int run();

private:
    client_ui::UIManager mUI;
    client_ui::ConnectionPanel mConnPanel;
    client_ui::TradingPanel mTradingPanel;
    client_ui::ChartPanel mChartPanel;
    client_ui::SimulatorPanel mSimPanel;
    client_ui::OrderBookPanel mBookPanel;
    
    admin::ExchangeManager mExchMgr;
    admin::ExchangePanel mExchPanel;

    std::unique_ptr<orderbook::OrderBook> mOrderBook;
    std::unique_ptr<ohlc::CandleAggregator> mAggregator;
    std::unique_ptr<simulator::StochasticSimulator> mSimulator;
    std::unique_ptr<data::MarketHistoryRepository> mHistoryRepo;
    
    asio::io_context mIoCtx;

    asio::executor_work_guard<asio::io_context::executor_type> mWork;
    std::thread mNetworkThread;

    std::shared_ptr<fix_client::FixClientSession> mFixSession;

    bool mLogicInitialized = false;
    trading_core::TradingCore* mCurrentCore = nullptr;
    void initLogic(trading_core::TradingCore& core);
};

} // namespace client_app
