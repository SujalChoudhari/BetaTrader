#pragma once

#include "orderbook/OrderBook.h"
#include "client_ui/MarketDataState.h"
#include <algorithm>
#include <cstdint>
#include <memory>

namespace client_ui {

    inline float depthFraction(std::uint64_t quantity,
                               std::uint64_t maximumQuantity) {
        if (quantity == 0 || maximumQuantity == 0) {
            return 0.0F;
        }

        const long double fraction =
            static_cast<long double>(quantity) /
            static_cast<long double>(maximumQuantity);
        return static_cast<float>(std::min(1.0L, fraction));
    }

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
