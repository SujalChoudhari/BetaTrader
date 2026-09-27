#include "client_ui/OrderTicket.h"
#include "client_ui/TradingPanel.h"
#include <gtest/gtest.h>

namespace client_ui {
    namespace {

        TEST(OrderTicketValidationTests, AcceptsLimitOrderWithRequiredPrice)
        {
            const OrderTicket ticket{"EURUSD", '1', 1.0850, 100, '2', '0'};

            EXPECT_FALSE(validateOrderTicket(ticket));
        }

        TEST(OrderTicketValidationTests, AcceptsMarketOrderWithoutLimitPrice)
        {
            const OrderTicket ticket{"EURUSD", '2', 0.0, 100, '1', '3'};

            EXPECT_FALSE(validateOrderTicket(ticket));
        }

        TEST(OrderTicketValidationTests, RejectsLimitOrderWithoutPositivePrice)
        {
            const OrderTicket ticket{"EURUSD", '1', 0.0, 100, '2', '0'};

            EXPECT_EQ(validateOrderTicket(ticket),
                      "Limit orders require a positive price.");
        }

        TEST(OrderTicketValidationTests, RejectsInvalidOrderFields)
        {
            EXPECT_EQ(validateOrderTicket({"", '1', 1.0, 100, '2', '0'}),
                      "Symbol is required.");
            EXPECT_EQ(validateOrderTicket({"NOTREAL", '1', 1.0, 100, '2', '0'}),
                      "Symbol is not supported.");
            EXPECT_EQ(validateOrderTicket({"EURUSD", '3', 1.0, 100, '2', '0'}),
                      "Side must be BUY or SELL.");
            EXPECT_EQ(validateOrderTicket({"EURUSD", '1', 1.0, 0, '2', '0'}),
                      "Quantity must be positive.");
            EXPECT_EQ(validateOrderTicket({"EURUSD", '1', 1.0, 100, '9', '0'}),
                      "Order type is unsupported.");
            EXPECT_EQ(validateOrderTicket({"EURUSD", '1', 1.0, 100, '2', '9'}),
                      "Time-in-force is unsupported.");
        }

        TEST(TradingPanelSubmissionTests,
             RejectsMissingSessionWithoutDereference)
        {
            TradingPanel panel;
            std::shared_ptr<fix_client::FixClientSession> session;

            EXPECT_FALSE(panel.trySubmitOrder(session, '1', '2', '0'));
            EXPECT_EQ(panel.validationMessage(),
                      "Session not active. Cannot trade.");
        }

    } // namespace
} // namespace client_ui
