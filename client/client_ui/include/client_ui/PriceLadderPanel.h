#pragma once

#include "client_orderbook/OrderBookManager.h"
#include <string>
#include <functional>
#include <memory>
#include <map>

namespace client_ui {

/**
 * @class PriceLadderPanel
 * @brief Renders multiple vertical price ladders (DOM) for trading.
 */
class PriceLadderPanel {
public:
    using OnClickCallback = std::function<void(const std::string& symbol, char side, double price, bool isOneClick, int notional)>;
    using OnSubscribeCallback = std::function<void(const std::string& symbol)>;

    PriceLadderPanel(client_orderbook::OrderBookManager& manager);

    /**
     * @brief Renders the Price Ladder window.
     */
    void render();

    /**
     * @brief Sets the callback for when a price level is clicked.
     */
    void setOnClickCallback(OnClickCallback cb) { mOnClickCb = std::move(cb); }
    void setOnSubscribeCallback(OnSubscribeCallback cb) { mOnSubscribeCb = std::move(cb); }

private:
    void renderInstrumentTile(std::shared_ptr<client_orderbook::OrderBookModel> model);
    void renderPrice(double price);

    client_orderbook::OrderBookManager& mManager;
    OnClickCallback mOnClickCb;
    OnSubscribeCallback mOnSubscribeCb;

    bool mOneClickMode = false;
    float mNotional = 1000000.0f;

    // Animation state: symbol -> price -> timestamp
    struct FlashState {
        double lastPrice;
        double flashTime;
    };
    std::map<std::string, std::map<double, double>> mPriceFlashMap;
};

} // namespace client_ui
