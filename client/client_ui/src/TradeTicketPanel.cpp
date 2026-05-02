#include "client_ui/TradeTicketPanel.h"
#include <imgui.h>
#include <iomanip>
#include <sstream>
#include "logging/Logger.h"

namespace client_ui {

TradeTicketPanel::TradeTicketPanel() {
    // Initialize price with a default or wait for pre-fill
}

void TradeTicketPanel::render(std::shared_ptr<fix_client::FixClientSession> session) {
    ImGui::Begin("Trade Ticket");

    // Symbol Selection
    if (ImGui::BeginCombo("Symbol", mSymbolBuffer)) {
        for (const auto& sym : common::symbol_names) {
            bool isSelected = (sym == mSymbolBuffer);
            if (ImGui::Selectable(sym.data(), isSelected)) {
                strncpy(mSymbolBuffer, sym.data(), sizeof(mSymbolBuffer));
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Side Selection
    ImGui::Text("Side");
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green for Buy
    ImGui::RadioButton("BUY", &mSide, 0);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red for Sell
    ImGui::RadioButton("SELL", &mSide, 1);
    ImGui::PopStyleColor();

    // Order Type
    const char* types[] = {"MARKET", "LIMIT"};
    ImGui::Combo("Type", &mOrderType, types, IM_ARRAYSIZE(types));

    // Quantity
    ImGui::InputInt("Quantity", &mQuantity, 10000, 100000);
    if (mQuantity < 0) mQuantity = 0;

    // Price
    bool isLimit = (mOrderType == 1);
    if (!isLimit) {
        ImGui::BeginDisabled();
    }
    ImGui::InputDouble("Price", &mPrice, 0.00001, 0.0001, "%.5f");
    if (!isLimit) {
        ImGui::EndDisabled();
    }

    ImGui::Separator();

    // Validation
    bool valid = (strlen(mSymbolBuffer) > 0) && (mQuantity > 0) && (!isLimit || mPrice > 0.0);
    bool isSessionActive = (session && session->getState() == fix_client::FixClientState::Active);

    if (!valid || !isSessionActive) {
        ImGui::BeginDisabled();
    }

    // Action Button
    ImVec4 btnColor = (mSide == 0) ? ImVec4(0.1f, 0.4f, 0.1f, 1.0f) : ImVec4(0.4f, 0.1f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
    if (ImGui::Button("SUBMIT ORDER", ImVec2(-FLT_MIN, 40))) {
        submitOrder(session);
    }
    ImGui::PopStyleColor();

    if (!valid || !isSessionActive) {
        ImGui::EndDisabled();
        if (!isSessionActive) {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Session must be Active to trade.");
        }
    }

    // Feedback
    if (!mFeedbackMsg.empty()) {
        double elapsed = ImGui::GetTime() - mFeedbackTimer;
        if (elapsed < 5.0) {
            float alpha = 1.0f;
            if (elapsed > 4.0) alpha = 1.0f - (float)(elapsed - 4.0);
            ImGui::TextColored(ImVec4(0, 1, 0, alpha), "%s", mFeedbackMsg.c_str());
        } else {
            mFeedbackMsg = "";
        }
    }

    ImGui::End();
}

void TradeTicketPanel::preFill(const std::string& symbol, char side, double price) {
    strncpy(mSymbolBuffer, symbol.c_str(), sizeof(mSymbolBuffer));
    mSide = (side == '1' || side == 'B') ? 0 : 1; // 1=Buy, 2=Sell in FIX
    mPrice = price;
    mOrderType = 1; // Limit
    
    // Bring to focus (This requires the window name to match)
    ImGui::SetWindowFocus("Trade Ticket");
}

void TradeTicketPanel::submitOrder(std::shared_ptr<fix_client::FixClientSession> session) {
    if (!session) return;

    char sideChar = (mSide == 0) ? '1' : '2'; // 1=Buy, 2=Sell
    char typeChar = (mOrderType == 0) ? '1' : '2'; // 1=Market, 2=Limit

    session->sendNewOrder(mSymbolBuffer, sideChar, mPrice, mQuantity, typeChar);

    mFeedbackMsg = "Order sent for " + std::string(mSymbolBuffer);
    mFeedbackTimer = ImGui::GetTime();
    LOG_INFO("TradeTicketPanel: Submitted {} order for {} Qty={}", (mSide == 0 ? "BUY" : "SELL"), mSymbolBuffer, mQuantity);
}

} // namespace client_ui
