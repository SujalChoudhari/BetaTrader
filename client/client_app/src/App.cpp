#include "App.h"
#include "logging/Logger.h"
#include <iostream>

namespace client_app {

App::App() : mWork(asio::make_work_guard(mIoCtx)), mExchMgr(), mExchPanel(mExchMgr), mPriceLadder(mOrderBookMgr) {

    // Wire Price Ladder -> Trade Ticket (with One-Click support)
    mPriceLadder.setOnClickCallback([this](const std::string& symbol, char side, double price, bool isOneClick, int notional) {
        if (isOneClick && mFixSession) {
            mFixSession->sendNewOrder(symbol, side, price, notional);
            LOG_INFO("One-Click Trade: Submitted {} order for {} Qty={} Price={}", 
                     (side == '1' ? "BUY" : "SELL"), symbol, notional, price);
        } else {
            mTradeTicketPanel.preFill(symbol, side, price);
        }
    });
    
    mPriceLadder.setOnSubscribeCallback([this](const std::string& symbol) {
        if (mFixSession) {
            mFixSession->sendMarketDataRequest(symbol);
        }
    });

    mOrderBlotter.setOnCancelCallback([this](const std::string& symbol, char side, uint64_t clOrdID, int qty) {
        if (mFixSession) {
            mFixSession->sendOrderCancelRequest(symbol, side, clOrdID, qty);
        }
    });

    mNetworkThread = std::thread([this]() {
        try {
            mIoCtx.run();
        } catch (const std::exception& e) {
            LOG_CRITICAL("Network thread exception: {}", e.what());
        }
    });
}

App::~App() {
    mWork.reset();
    mIoCtx.stop();

    if (mNetworkThread.joinable()) {
        mNetworkThread.join();
    }
    mUI.stop();
}

int App::run() {
    logging::Logger::Init("client_app", "logs/client_app.log", true, true);
    LOG_INFO("Starting BetaTrader Client...");

    if (!mUI.start("BetaTrader Client Dashboard", 1280, 720)) {
        return 1;
    }

    while (!mUI.shouldClose()) {
        mUI.beginFrame();
        mUI.renderDockspace();

        // Render Panels
        if (mFixSession && mFixSession.get() != mLastSessionPtr) {
            mLastSessionPtr = mFixSession.get();
            mOrderBookMgr.clearAll();
            mFixSession->setMessageCallback([this](const fix_client::ParsedFixMessage& msg) {
                onFixMessage(msg);
            });
            mFixSession->setRawMessageCallback([this](const std::string& msg, bool isInbound) {
                mConnPanel.addLog(isInbound ? "IN" : "OUT", msg);
            });
        }

        mConnPanel.render(mFixSession, mIoCtx);

        // Auto-subscribe to all existing models when session becomes active
        if (mFixSession) {
            auto currentState = mFixSession->getState();
            if (currentState == fix_client::FixClientState::Active && 
                mLastSessionState != fix_client::FixClientState::Active) {
                
                LOG_INFO("Session became Active. Auto-subscribing to all instruments...");
                auto models = mOrderBookMgr.getAllModels();
                for (auto& model : models) {
                    mFixSession->sendMarketDataRequest(model->getSymbol());
                }
            }
            mLastSessionState = currentState;
        }

        mExchPanel.render();
        mTradeTicketPanel.render(mFixSession);
        mPriceLadder.render();
        mOrderBlotter.render();


        mUI.endFrame();
    }

    LOG_INFO("Shutting down BetaTrader Client...");
    return 0;
}

void App::onFixMessage(const fix_client::ParsedFixMessage& msg) {
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, fix::MarketDataSnapshotFullRefresh>) {
            auto model = mOrderBookMgr.getModel(common::to_string(arg.symbol));
            std::vector<client_orderbook::PriceLevel> bids, asks;
            for (const auto& entry : arg.entries) {
                if (entry.entryType == fix::MDEntryType::Bid) {
                    bids.push_back({entry.price, (uint64_t)entry.size, 1});
                } else if (entry.entryType == fix::MDEntryType::Offer) {
                    asks.push_back({entry.price, (uint64_t)entry.size, 1});
                }
            }
            model->applySnapshot(bids, asks);
        } else if constexpr (std::is_same_v<T, fix::MarketDataIncrementalRefresh>) {
            auto model = mOrderBookMgr.getModel(common::to_string(arg.symbol));
            for (const auto& entry : arg.entries) {
                bool isBid = (entry.entryType == fix::MDEntryType::Bid);
                model->updateLevel(isBid, entry.price, entry.size, (int)entry.updateAction);
            }
        } else if constexpr (std::is_same_v<T, fix::ExecutionReport>) {
            LOG_INFO("ExecutionReport received: OrderID={} Symbol={} Status={} LastPx={} LastQty={}", 
                     arg.getExchangeOrderId(), common::to_string(arg.getSymbol()), static_cast<int>(arg.getStatus()), arg.getLastPrice(), arg.getLastQuantity());
            mOrderBlotter.addReport(arg);
        }
    }, msg);
}

} // namespace client_app
