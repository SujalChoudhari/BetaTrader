#pragma once

#include "client_ui/UIManager.h"
#include "fix_client/ConnectionPanel.h"
#include "admin/ExchangeManager.h"
#include "admin/ExchangePanel.h"
#include "client_ui/TradeTicketPanel.h"
#include "client_ui/PriceLadderPanel.h"
#include "client_ui/OrderBlotterPanel.h"
#include "client_orderbook/OrderBookManager.h"
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
    void onFixMessage(const fix_client::ParsedFixMessage& msg);

    // Testing Helpers
    client_orderbook::OrderBookManager& getOrderBookManager() { return mOrderBookMgr; }
    client_ui::TradeTicketPanel& getTradeTicketPanel() { return mTradeTicketPanel; }

private:
    client_ui::UIManager mUI;
    fix_client::ConnectionPanel mConnPanel;
    
    admin::ExchangeManager mExchMgr;
    admin::ExchangePanel mExchPanel;
    client_ui::TradeTicketPanel mTradeTicketPanel;
    client_ui::OrderBlotterPanel mOrderBlotter;
    client_orderbook::OrderBookManager mOrderBookMgr;
    client_ui::PriceLadderPanel mPriceLadder;
    
    asio::io_context mIoCtx;

    asio::executor_work_guard<asio::io_context::executor_type> mWork;
    std::thread mNetworkThread;

    std::shared_ptr<fix_client::FixClientSession> mFixSession;
    fix_client::FixClientSession* mLastSessionPtr = nullptr;
    fix_client::FixClientState mLastSessionState = fix_client::FixClientState::Disconnected;

    friend class AppLogicTests;
};

} // namespace client_app
