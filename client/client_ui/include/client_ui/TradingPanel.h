#pragma once

#include "fix_client/FixClientSession.h"
#include <memory>
#include <string>

namespace client_ui {

    class TradingPanel {
    public:
        TradingPanel();
        ~TradingPanel() = default;

        void render(std::shared_ptr<fix_client::FixClientSession>& session);

        [[nodiscard]] bool trySubmitOrder(
                const std::shared_ptr<fix_client::FixClientSession>& session,
                char side, char orderType, char timeInForce);
        [[nodiscard]] const std::string& validationMessage() const
        {
            return mValidationMessage;
        }

    private:
        char mSymbol[16] = "EURUSD";
        double mPrice = 1.0850;
        int mQuantity = 100;
        char mOrdType = '2'; // Limit
        char mTif = '0'; // Day
        std::string mValidationMessage;
    };

} // namespace client_ui
