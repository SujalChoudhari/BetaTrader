#pragma once

#include "fix_client/FixClientSession.h"
#include <string>
#include <memory>

namespace client_ui {

    class TradingPanel {
    public:
        TradingPanel();
        ~TradingPanel() = default;

        void render(std::shared_ptr<fix_client::FixClientSession>& session);

    private:
        char mSymbol[16] = "EURUSD";
        double mPrice = 1.0850;
        int mQuantity = 100;
        char mOrdType = '2'; // Limit
        char mTif = '0';     // Day
    };

} // namespace client_ui
