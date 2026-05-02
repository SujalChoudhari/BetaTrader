#pragma once

#include "common_fix/ExecutionReport.h"
#include <vector>
#include <string>
#include <mutex>
#include <functional>

namespace client_ui {

/**
 * @class OrderBlotterPanel
 * @brief Displays a list of all order execution reports and status updates.
 */
class OrderBlotterPanel {
public:
    struct OrderEntry {
        uint64_t clientOrderId;
        uint64_t exchangeOrderId;
        std::string symbol;
        std::string side;
        uint64_t qty;
        double price;
        std::string status;
        std::string text;
        std::string timestamp;
    };

    using CancelCallback = std::function<void(const std::string& symbol, char side, uint64_t clOrdID, int qty)>;

    OrderBlotterPanel();
    void render();
    void addReport(const fix::ExecutionReport& report);
    void setOnCancelCallback(CancelCallback cb) { mOnCancelCb = cb; }

private:
    std::vector<OrderEntry> mOrders;
    std::mutex mMutex;
    CancelCallback mOnCancelCb;
};

} // namespace client_ui
