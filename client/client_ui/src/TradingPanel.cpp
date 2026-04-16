#include "client_ui/TradingPanel.h"
#include <imgui.h>

namespace client_ui {

    TradingPanel::TradingPanel() {}

    void TradingPanel::render(std::shared_ptr<fix_client::FixClientSession>& session) {
        ImGui::Begin("Order Entry");

        ImGui::InputText("Symbol", mSymbol, sizeof(mSymbol));
        ImGui::InputDouble("Price", &mPrice, 0.0001, 0.001, "%.4f");
        ImGui::InputInt("Quantity", &mQuantity);

        bool isActive = session && session->getState() == fix_client::FixClientState::Active;

        if (!isActive) ImGui::BeginDisabled();

        if (ImGui::Button("BUY", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 40))) {
            session->sendNewOrder(mSymbol, '1', mPrice, mQuantity);
        }
        ImGui::SameLine();
        if (ImGui::Button("SELL", ImVec2(-1, 40))) {
            session->sendNewOrder(mSymbol, '2', mPrice, mQuantity);
        }

        if (!isActive) {
            ImGui::EndDisabled();
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Session not active. Cannot trade.");
        }

        ImGui::Separator();
        ImGui::Text("Quick Actions");
        if (ImGui::Button("Market Buy")) {
            session->sendNewOrder(mSymbol, '1', 0, mQuantity, '1', '3'); // Market, IOC
        }
        ImGui::SameLine();
        if (ImGui::Button("Market Sell")) {
            session->sendNewOrder(mSymbol, '2', 0, mQuantity, '1', '3'); // Market, IOC
        }

        ImGui::End();
    }

} // namespace client_ui
