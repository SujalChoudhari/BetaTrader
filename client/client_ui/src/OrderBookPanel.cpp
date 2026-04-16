#include "client_ui/OrderBookPanel.h"
#include <imgui.h>

namespace client_ui {

    OrderBookPanel::OrderBookPanel() {}

    OrderBookPanel::~OrderBookPanel() {}

    void OrderBookPanel::render(const orderbook::OrderBook* book) {
        ImGui::Begin("Order Book (L2)");

        if (!book) {
            ImGui::Text("Enter a symbol and start the data aggregator to view depth.");
            ImGui::End();
            return;
        }

        auto ss = book->getUISnapshot(mMaxDepth);

        ImGui::Text("Spread: %.4f | Mid: %.4f", ss.spread, ss.midPrice);
        ImGui::Separator();

        if (ImGui::BeginTable("BookLadder", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Size");
            ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Depth");
            ImGui::TableHeadersRow();

            // ASKS (reverse order to show them on top)
            for (int i = (int)ss.asks.size() - 1; i >= 0; --i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%llu", (unsigned long long)ss.asks[i].qty);
                
                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%.4f", ss.asks[i].price);
                
                ImGui::TableSetColumnIndex(2);
                // Simple visualizer for depth
                ImGui::ProgressBar(0.2f, ImVec2(-1, 0), "");
            }

            // SPREAD ROW
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(1);
            ImGui::Separator();

            // BIDS
            for (size_t i = 0; i < ss.bids.size(); ++i) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%llu", (unsigned long long)ss.bids[i].qty);
                
                ImGui::TableSetColumnIndex(1);
                ImGui::TextColored(ImVec4(0.4f, 1, 0.4f, 1), "%.4f", ss.bids[i].price);
                
                ImGui::TableSetColumnIndex(2);
                ImGui::ProgressBar(0.2f, ImVec2(-1, 0), "");
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }

} // namespace client_ui
