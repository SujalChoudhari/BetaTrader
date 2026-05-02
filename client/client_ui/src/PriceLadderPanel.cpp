#include "client_ui/PriceLadderPanel.h"
#include "common/Instrument.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>
#include <vector>

namespace client_ui {

PriceLadderPanel::PriceLadderPanel(client_orderbook::OrderBookManager& manager) 
    : mManager(manager) {}

void PriceLadderPanel::render() {
    ImGui::Begin("Price Ladder");

    // Toolbar
    ImGui::SetNextItemWidth(150);
    ImGui::InputFloat("Notional", &mNotional, 10000.0f, 100000.0f, "%.0f");
    ImGui::SameLine();
    
    bool stylePushed = false;
    if (mOneClickMode) {
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
        stylePushed = true;
    }
    ImGui::Checkbox("One-Click Trading", &mOneClickMode);
    if (stylePushed) {
        ImGui::PopStyleColor(2);
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    if (ImGui::Button("+ Add Symbol", ImVec2(100, 0))) {
        ImGui::OpenPopup("AddSymbolPopup");
    }

    if (ImGui::BeginPopup("AddSymbolPopup")) {
        for (const auto& symName : common::symbol_names) {
            if (ImGui::Selectable(symName.data())) {
                std::string sym(symName);
                mManager.getModel(sym);
                if (mOnSubscribeCb) mOnSubscribeCb(sym);
            }
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Horizontal scroll container for tiles
    if (ImGui::BeginChild("##ladder_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        auto models = mManager.getAllModels();
        for (auto& model : models) {
            renderInstrumentTile(model);
            ImGui::SameLine();
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void PriceLadderPanel::renderInstrumentTile(std::shared_ptr<client_orderbook::OrderBookModel> model) {
    ImGui::PushID(model->getSymbol().c_str());
    
    ImGui::BeginGroup();
    
    // Header
    ImGui::TextDisabled("FX Spot");
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Large font if available
    ImGui::Text("%s", model->getSymbol().c_str());
    ImGui::PopFont();

    // Stats
    auto [bestBid, bestAsk] = model->getTopOfBook();
    float spread = 0.0f;
    if (bestBid && bestAsk) spread = (bestAsk->price - bestBid->price) * 10000.0f;
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Spread: %.1f pips", spread);

    // Table
    static ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
    if (ImGui::BeginTable("##ladder_table", 3, flags, ImVec2(280, 0))) {
        ImGui::TableSetupColumn("Buy", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Spread", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Sell", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableHeadersRow();

        auto bids = model->getBids(6);
        auto asks = model->getAsks(6);

        int rowCount = 6; // Fixed 6 rows
        for (int i = 0; i < rowCount; ++i) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, 45.0f); // Taller rows

            // Buy Column (Bid)
            ImGui::TableSetColumnIndex(0);
            if (i < (int)bids.size()) {
                auto& b = bids[i];
                ImGui::PushID(i * 2);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.25f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.4f, 0.2f, 1.0f));
                if (ImGui::Button("##bid_btn", ImVec2(-1, 40))) {
                    if (mOnClickCb) mOnClickCb(model->getSymbol(), '1', b.price, mOneClickMode, (int)mNotional);
                }
                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMin().x + 5, ImGui::GetItemRectMin().y + 5));
                renderPrice(b.price);
                ImGui::PopStyleColor(2);
                ImGui::PopID();
            }

            // Spread Column
            ImGui::TableSetColumnIndex(1);
            if (i < (int)bids.size() && i < (int)asks.size()) {
                float sprd = (asks[i].price - bids[i].price) * 10000.0f;
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12);
                ImGui::TextDisabled(" %.1f", sprd);
                
                // Mini volume bar
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImGui::GetCursorScreenPos();
                dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 50, p.y + 4), IM_COL32(60, 60, 60, 255));
                dl->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 25, p.y + 4), IM_COL32(200, 200, 200, 255));
            }

            // Sell Column (Ask)
            ImGui::TableSetColumnIndex(2);
            if (i < (int)asks.size()) {
                auto& a = asks[i];
                ImGui::PushID(i * 2 + 1);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("##ask_btn", ImVec2(-1, 40))) {
                    if (mOnClickCb) mOnClickCb(model->getSymbol(), '2', a.price, mOneClickMode, (int)mNotional);
                }
                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMin().x + 5, ImGui::GetItemRectMin().y + 5));
                renderPrice(a.price);
                ImGui::PopStyleColor(2);
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }

    ImGui::EndGroup();
    ImGui::PopID();
}

void PriceLadderPanel::renderPrice(double price) {
    std::string s = std::to_string(price);
    size_t dot = s.find('.');
    if (dot == std::string::npos) {
        ImGui::Text("%.5f", price);
        return;
    }

    // EURUSD: 1.08 53 0
    std::string bigFigure = s.substr(0, dot + 3); 
    std::string pips = s.substr(dot + 3, 2);      
    std::string pipette = s.substr(dot + 5, 1);   

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 0));
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGui::TextDisabled("%s", bigFigure.c_str());
    ImGui::SameLine();
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%s", pips.c_str()); 
    ImGui::SameLine();
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    ImGui::TextDisabled("%s", pipette.c_str());
    
    ImGui::PopStyleVar();
}

} // namespace client_ui
