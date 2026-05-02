#include "common/Instrument.h"
#include "fix_client/FixClientSession.h"
#include <string>
#include <memory>
#include <vector>

namespace client_ui {

/**
 * @class TradeTicketPanel
 * @brief Handles manual order entry and pre-fill from market data.
 */
class TradeTicketPanel {
public:
    TradeTicketPanel();

    /**
     * @brief Renders the Trade Ticket window.
     * @param session The active FIX session to send orders through.
     */
    void render(std::shared_ptr<fix_client::FixClientSession> session);

    /**
     * @brief Pre-fills the ticket with values from a ladder click.
     */
    void preFill(const std::string& symbol, char side, double price);

private:
    void submitOrder(std::shared_ptr<fix_client::FixClientSession> session);

    // Order State
    char mSymbolBuffer[16] = "EURUSD";
    int mSide = 0;        // 0 = Buy, 1 = Sell
    int mOrderType = 1;   // 0 = Market, 1 = Limit
    int mQuantity = 1000000;
    double mPrice = 0.0;

    // UI Feedback
    std::string mFeedbackMsg;
    double mFeedbackTimer = 0.0;
    bool mShowError = false;
};

} // namespace client_ui
