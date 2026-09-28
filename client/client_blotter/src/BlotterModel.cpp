#include "blotter/BlotterModel.h"
#include <mutex>
#include <sstream>

namespace client_blotter {
    namespace {

        std::string executionKey(const fix::ExecutionReport& report)
        {
            std::ostringstream key;
            key << report.getExecutionId() << '|' << report.getClientOrderId()
                << '|' << common::to_string(report.getStatus()) << '|'
                << report.getCumulativeQuantity() << '|'
                << report.getLeavesQuantity() << '|' << report.getLastPrice()
                << '|' << report.getLastQuantity() << '|' << report.getText();
            return key.str();
        }

    } // namespace

    void BlotterModel::recordOrderIntent(const OrderIntent& intent)
    {
        std::unique_lock lock(mMutex);
        auto& row = mOrders[intent.clientOrderId];
        row.clientOrderId = intent.clientOrderId;
        row.symbol = intent.symbol;
        row.side = intent.side;
        row.orderType = intent.orderType;
        row.orderTypeKnown = true;
        row.price = intent.price;
        row.originalQuantity = intent.originalQuantity;
        row.remainingQuantity = intent.originalQuantity;
        row.status = common::OrderStatus::New;
        row.reason.clear();
    }

    void BlotterModel::processExecution(const fix::ExecutionReport& report)
    {
        std::unique_lock lock(mMutex);

        const auto& executionId = report.getExecutionId();
        if (!executionId.empty()
            && !mSeenExecutionKeys.emplace(executionKey(report)).second) {
            return;
        }

        const auto& clientOrderId = report.getClientOrderId();
        auto [it, inserted] = mOrders.try_emplace(clientOrderId);
        auto& row = it->second;
        if (inserted) {
            row.clientOrderId = clientOrderId;
            row.symbol = report.getSymbol();
            row.side = report.getSide();
            row.price = report.getLastPrice();
            row.originalQuantity = report.getOrderQuantity();
        }

        row.symbol = report.getSymbol();
        row.side = report.getSide();
        row.exchangeOrderId = report.getExchangeOrderId();
        row.status = report.getStatus();
        row.remainingQuantity = report.getLeavesQuantity();
        row.originalQuantity = report.getOrderQuantity() == 0
                                       ? row.originalQuantity
                                       : report.getOrderQuantity();
        if (!row.orderTypeKnown && report.getLastPrice() != 0.0) {
            row.price = report.getLastPrice();
        }
        row.reason = report.getText();

        mHistory.push_back({clientOrderId, report.getExchangeOrderId(),
                            report.getSymbol(), report.getSide(),
                            report.getStatus(), report.getTransactionTime(),
                            report.getLastPrice(), report.getLastQuantity(),
                            report.getText()});

        switch (report.getStatus()) {
        case common::OrderStatus::New:
        case common::OrderStatus::PartiallyFilled:
            break;
        case common::OrderStatus::Filled:
        case common::OrderStatus::Cancelled:
        case common::OrderStatus::Rejected:
            mOrders.erase(it);
            break;
        }
    }

    std::vector<OrderRow> BlotterModel::activeOrders() const
    {
        std::shared_lock lock(mMutex);
        std::vector<OrderRow> result;
        result.reserve(mOrders.size());
        for (const auto& [clientOrderId, row]: mOrders) {
            (void)clientOrderId;
            result.push_back(row);
        }
        return result;
    }

    std::vector<HistoryRow> BlotterModel::history() const
    {
        std::shared_lock lock(mMutex);
        return mHistory;
    }

} // namespace client_blotter
