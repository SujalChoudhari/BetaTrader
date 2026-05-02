#pragma once

#include <client_orderbook/OrderBookModel.h>

namespace client_ui {

/**
 * @class OrderBookView
 * @brief Renders the L2 Orderbook using Dear ImGui.
 */
class OrderBookView {
public:
    OrderBookView(const client_orderbook::OrderBookModel& model);

    /**
     * @brief Renders the orderbook window.
     */
    void render();

private:
    const client_orderbook::OrderBookModel& mModel;
};

} // namespace client_ui
