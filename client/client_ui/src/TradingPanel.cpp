#include "client_ui/TradingPanel.h"
#include "client_ui/OrderTicket.h"
#include <imgui.h>

namespace client_ui {

    TradingPanel::TradingPanel() {}

    bool TradingPanel::trySubmitOrder(
            const std::shared_ptr<fix_client::FixClientSession>& session,
            char side, char orderType, char timeInForce)
    {
        const OrderTicket ticket{mSymbol,   side,      mPrice,
                                 mQuantity, orderType, timeInForce};
        if (const auto validationError = validateOrderTicket(ticket)) {
            mValidationMessage = *validationError;
            return false;
        }

        if (!session
            || session->getState() != fix_client::FixClientState::Active) {
            mValidationMessage = "Session not active. Cannot trade.";
            return false;
        }

        session->sendNewOrder(ticket.symbol, ticket.side, ticket.price,
                              ticket.quantity, ticket.orderType,
                              ticket.timeInForce);
        mValidationMessage.clear();
        return true;
    }

    void
    TradingPanel::render(std::shared_ptr<fix_client::FixClientSession>& session)
    {
        ImGui::Begin("Order Entry");

        ImGui::InputText("Symbol", mSymbol, sizeof(mSymbol));
        ImGui::InputDouble("Price", &mPrice, 0.0001, 0.001, "%.4f");
        ImGui::InputInt("Quantity", &mQuantity);

        const char* orderTypes[] = {"Limit", "Market"};
        int orderTypeIndex = mOrdType == '1' ? 1 : 0;
        if (ImGui::Combo("Order Type", &orderTypeIndex, orderTypes,
                         IM_ARRAYSIZE(orderTypes))) {
            mOrdType = orderTypeIndex == 1 ? '1' : '2';
        }

        const char* timeInForceOptions[] = {"Day", "GTC", "IOC", "FOK"};
        int timeInForceIndex = mTif == '1'   ? 1
                               : mTif == '3' ? 2
                               : mTif == '4' ? 3
                                             : 0;
        if (ImGui::Combo("Time in Force", &timeInForceIndex, timeInForceOptions,
                         IM_ARRAYSIZE(timeInForceOptions))) {
            constexpr char timeInForceValues[] = {'0', '1', '3', '4'};
            mTif = timeInForceValues[timeInForceIndex];
        }

        const bool isActive
                = session
                  && session->getState() == fix_client::FixClientState::Active;
        if (!isActive) ImGui::BeginDisabled();

        if (ImGui::Button("BUY", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f,
                                        40))) {
            (void)trySubmitOrder(session, '1', mOrdType, mTif);
        }
        ImGui::SameLine();
        if (ImGui::Button("SELL", ImVec2(-1, 40))) {
            (void)trySubmitOrder(session, '2', mOrdType, mTif);
        }

        if (!isActive) {
            ImGui::EndDisabled();
            ImGui::TextColored(ImVec4(1, 0, 0, 1),
                               "Session not active. Cannot trade.");
        }

        ImGui::Separator();
        ImGui::Text("Quick Actions");
        if (!isActive) ImGui::BeginDisabled();
        if (ImGui::Button("Market Buy")) {
            (void)trySubmitOrder(session, '1', '1', '3'); // Market, IOC
        }
        ImGui::SameLine();
        if (ImGui::Button("Market Sell")) {
            (void)trySubmitOrder(session, '2', '1', '3'); // Market, IOC
        }
        if (!isActive) ImGui::EndDisabled();

        if (!mValidationMessage.empty()) {
            ImGui::TextColored(ImVec4(1, 0.6f, 0, 1), "%s",
                               mValidationMessage.c_str());
        }

        ImGui::End();
    }

} // namespace client_ui
