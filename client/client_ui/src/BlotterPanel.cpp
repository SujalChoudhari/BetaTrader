#include "client_ui/BlotterPanel.h"
#include <ctime>
#include <imgui.h>
#include <iomanip>
#include <sstream>
#include <string>

namespace client_ui {
    namespace {

        std::string formatTimestamp(const common::Timestamp& timestamp)
        {
            const auto time = std::chrono::system_clock::to_time_t(timestamp);
            std::tm localTime{};
            localtime_r(&time, &localTime);

            std::ostringstream result;
            result << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
            return result.str();
        }

        const char* reasonOrDash(const std::string& reason)
        {
            return reason.empty() ? "-" : reason.c_str();
        }

    } // namespace

    void BlotterPanel::render(const client_blotter::BlotterModel& model) const
    {
        const auto openOrders = model.activeOrders();
        ImGui::Begin("Open Orders");
        if (openOrders.empty()) { ImGui::TextDisabled("No open orders"); }
        else if (ImGui::BeginTable("OpenOrdersTable", 8,
                                   ImGuiTableFlags_Borders
                                           | ImGuiTableFlags_RowBg
                                           | ImGuiTableFlags_ScrollX
                                           | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Symbol");
            ImGui::TableSetupColumn("Side");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Price");
            ImGui::TableSetupColumn("Original");
            ImGui::TableSetupColumn("Remaining");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Client ID");
            ImGui::TableHeadersRow();

            for (const auto& order: openOrders) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(common::to_string(order.symbol).c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(common::to_string(order.side).c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(
                        order.orderTypeKnown
                                ? common::to_string(order.orderType).c_str()
                                : "Unknown");
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.5f", order.price);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%llu", static_cast<unsigned long long>(
                                            order.originalQuantity));
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%llu", static_cast<unsigned long long>(
                                            order.remainingQuantity));
                ImGui::TableSetColumnIndex(6);
                ImGui::TextUnformatted(common::to_string(order.status).c_str());
                ImGui::TableSetColumnIndex(7);
                ImGui::TextUnformatted(order.clientOrderId.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::End();

        const auto history = model.history();
        ImGui::Begin("Execution History");
        if (history.empty()) { ImGui::TextDisabled("No execution reports"); }
        else if (ImGui::BeginTable("ExecutionHistoryTable", 9,
                                   ImGuiTableFlags_Borders
                                           | ImGuiTableFlags_RowBg
                                           | ImGuiTableFlags_ScrollX
                                           | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Timestamp");
            ImGui::TableSetupColumn("Symbol");
            ImGui::TableSetupColumn("Side");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Price");
            ImGui::TableSetupColumn("Quantity");
            ImGui::TableSetupColumn("Order ID");
            ImGui::TableSetupColumn("Client ID");
            ImGui::TableSetupColumn("Reason");
            ImGui::TableHeadersRow();

            for (const auto& event: history) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                const auto timestamp = formatTimestamp(event.timestamp);
                ImGui::TextUnformatted(timestamp.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(common::to_string(event.symbol).c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(common::to_string(event.side).c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::TextUnformatted(common::to_string(event.status).c_str());
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%.5f", event.lastPrice);
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%llu", static_cast<unsigned long long>(
                                            event.lastQuantity));
                ImGui::TableSetColumnIndex(6);
                ImGui::Text("%llu", static_cast<unsigned long long>(
                                            event.exchangeOrderId));
                ImGui::TableSetColumnIndex(7);
                ImGui::TextUnformatted(event.clientOrderId.c_str());
                ImGui::TableSetColumnIndex(8);
                ImGui::TextUnformatted(reasonOrDash(event.reason));
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

} // namespace client_ui
