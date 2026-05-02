#include <client_ui/OrderBookView.h>
#include <imgui.h>
#include <iomanip>
#include <sstream>

namespace client_ui {

OrderBookView::OrderBookView(const client_orderbook::OrderBookModel& model) 
    : mModel(model) {}

void OrderBookView::render() {
    ImGui::Begin(("Orderbook: " + mModel.getSymbol()).c_str());

    auto bids = mModel.getBids(15);
    auto asks = mModel.getAsks(15);

    if (ImGui::BeginTable("OrderBookTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Quantity", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();

        // Find max quantity for depth bar scaling
        uint64_t maxQty = 1;
        for (const auto& pl : bids) if (pl.quantity > maxQty) maxQty = pl.quantity;
        for (const auto& pl : asks) if (pl.quantity > maxQty) maxQty = pl.quantity;

        size_t rows = std::max(bids.size(), asks.size());
        for (size_t i = 0; i < rows; ++i) {
            ImGui::TableNextRow();

            // Bid Column
            ImGui::TableSetColumnIndex(0);
            if (i < bids.size()) {
                const auto& pl = bids[i];
                float fraction = static_cast<float>(pl.quantity) / maxQty;
                
                // Render custom depth bar in background
                ImVec2 pos = ImGui::GetCursorScreenPos();
                float width = ImGui::GetContentRegionAvail().x;
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImVec2(pos.x + width * (1.0f - fraction), pos.y),
                    ImVec2(pos.x + width, pos.y + ImGui::GetTextLineHeightWithSpacing()),
                    IM_COL32(0, 255, 0, 40)
                );
                
                ImGui::Text("%llu", pl.quantity);
            }

            // Price Column
            ImGui::TableSetColumnIndex(1);
            if (i < bids.size() || i < asks.size()) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(5);
                if (i < bids.size() && i < asks.size()) {
                    ss << bids[i].price << " | " << asks[i].price;
                } else if (i < bids.size()) {
                    ss << bids[i].price << " | ---";
                } else {
                    ss << "--- | " << asks[i].price;
                }
                ImGui::TextUnformatted(ss.str().c_str());
            }

            // Ask Column
            ImGui::TableSetColumnIndex(2);
            if (i < asks.size()) {
                const auto& pl = asks[i];
                float fraction = static_cast<float>(pl.quantity) / maxQty;
                
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(
                    pos,
                    ImVec2(pos.x + ImGui::GetContentRegionAvail().x * fraction, pos.y + ImGui::GetTextLineHeightWithSpacing()),
                    IM_COL32(255, 0, 0, 40)
                );
                
                ImGui::Text("%llu", pl.quantity);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace client_ui
