#include "client_ui/OrderBlotterPanel.h"
#include <imgui.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace client_ui {

OrderBlotterPanel::OrderBlotterPanel() {}

void OrderBlotterPanel::render() {
    ImGui::Begin("Order Blotter");

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    
    if (ImGui::BeginTable("##blotter_table", 9, flags)) {
        ImGui::TableSetupColumn("Time");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Symbol");
        ImGui::TableSetupColumn("Side");
        ImGui::TableSetupColumn("Qty");
        ImGui::TableSetupColumn("Price");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("Text");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();

        std::lock_guard<std::mutex> lock(mMutex);
        for (auto it = mOrders.rbegin(); it != mOrders.rend(); ++it) {
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%s", it->timestamp.c_str());
            
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%llu", it->clientOrderId);
            
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", it->symbol.c_str());
            
            ImGui::TableSetColumnIndex(3);
            if (it->side == "BUY") ImGui::TextColored(ImVec4(0, 1, 0, 1), "BUY");
            else ImGui::TextColored(ImVec4(1, 0, 0, 1), "SELL");
            
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%llu", it->qty);
            
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.5f", it->price);
            
            ImGui::TableSetColumnIndex(6);
            ImVec4 statusColor = ImVec4(1, 1, 1, 1);
            if (it->status == "Filled") statusColor = ImVec4(0, 1, 0, 1);
            else if (it->status == "Rejected") statusColor = ImVec4(1, 0, 0, 1);
            else if (it->status == "New") statusColor = ImVec4(1, 1, 0, 1);
            
            ImGui::TextColored(statusColor, "%s", it->status.c_str());
            
            ImGui::TableSetColumnIndex(7);
            ImGui::TextWrapped("%s", it->text.c_str());

            ImGui::TableSetColumnIndex(8);
            if (it->status == "New" || it->status == "Partial") {
                ImGui::PushID(it->clientOrderId);
                if (ImGui::Button("Cancel")) {
                    if (mOnCancelCb) {
                        char sideCode = (it->side == "BUY") ? '1' : '2';
                        mOnCancelCb(it->symbol, sideCode, it->clientOrderId, it->qty);
                    }
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void OrderBlotterPanel::addReport(const fix::ExecutionReport& report) {
    std::lock_guard<std::mutex> lock(mMutex);
    
    OrderEntry entry;
    entry.clientOrderId = report.getClientOrderId();
    entry.exchangeOrderId = report.getExchangeOrderId();
    entry.symbol = common::to_string(report.getSymbol());
    entry.side = (report.getSide() == common::OrderSide::Buy) ? "BUY" : "SELL";
    entry.qty = report.getOrderQuantity();
    entry.price = report.getLastPrice() > 0 ? report.getLastPrice() : 0.0;

    switch (report.getStatus()) {
        case common::OrderStatus::New: entry.status = "New"; break;
        case common::OrderStatus::PartiallyFilled: entry.status = "Partial"; break;
        case common::OrderStatus::Filled: entry.status = "Filled"; break;
        case common::OrderStatus::Cancelled: entry.status = "Cancelled"; break;
        case common::OrderStatus::Rejected: entry.status = "Rejected"; break;
        default: entry.status = "Unknown"; break;
    }
    
    entry.text = report.getText();
    
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
    entry.timestamp = ss.str();
    
    // Update existing order if it's the same ID, otherwise add new
    bool found = false;
    for (auto& existing : mOrders) {
        if (existing.clientOrderId == entry.clientOrderId && entry.clientOrderId != 0) {
            existing.status = entry.status;
            existing.text = entry.text;
            existing.exchangeOrderId = entry.exchangeOrderId;
            if (entry.price > 0) existing.price = entry.price;
            found = true;
            break;
        }
    }
    
    if (!found) {
        mOrders.push_back(entry);
        if (mOrders.size() > 1000) mOrders.erase(mOrders.begin());
    }
}

} // namespace client_ui
