#pragma once

#include "orderbook/OrderBook.h"
#include "client_ui/MarketDataState.h"
#include <memory>

namespace client_ui {

    /**
     * @class OrderBookPanel
     * @brief Visualizes the L2 market depth ladder.
     */
    class OrderBookPanel {
    public:
        OrderBookPanel();
        ~OrderBookPanel();

        void render(const orderbook::OrderBook* book, const MarketDataState& marketData);

    private:
        int mMaxDepth = 10;
    };

} // namespace client_ui
