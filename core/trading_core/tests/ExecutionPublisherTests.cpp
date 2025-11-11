#include "common/Order.h"
#include "common/Trade.h"
#include "trading_core/ExecutionPublisher.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

// Helper class to temporarily redirect cout
class CoutRedirector {
public:
    CoutRedirector(std::stringstream& new_buffer)
        : old_buffer(std::cout.rdbuf(new_buffer.rdbuf()))
    {}

    ~CoutRedirector() { std::cout.rdbuf(old_buffer); }

private:
    std::streambuf* old_buffer;
};

TEST(ExecutionPublisherTests, PublishExecution)
{
    common::Order order(123, common::Instrument::EURUSD, "client1",
                        common::OrderSide::Buy, common::OrderType::Limit,
                        common::TimeInForce::DAY, 100, 1.25,
                        std::chrono::system_clock::now());
    order.setRemainingQuantity(50);
    std::string action = "PARTIAL_FILL";

    std::stringstream buffer;
    {
        CoutRedirector redirect(buffer);
        trading_core::ExecutionPublisher::publishExecution(order, action);
    }

    std::string expected_output
            = "[ExecutionPublisher] EXECUTION | Action=PARTIAL_FILL | "
              "OrderID=123 | Symbol=EURUSD | Qty=50 | Price=1.250000 | "
              "Client=client1\n";
    ASSERT_EQ(buffer.str(), expected_output);
}

TEST(ExecutionPublisherTests, PublishTrade)
{
    common::Trade trade(789, common::Instrument::GBPUSD, 456, 123, 50, 1.2505,
                        std::chrono::system_clock::now());
    auto timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                trade.getTimestamp().time_since_epoch())
                                .count();

    std::stringstream buffer;
    {
        CoutRedirector redirect(buffer);
        trading_core::ExecutionPublisher::publishTrade(trade);
    }

    std::stringstream expected_output_stream;
    expected_output_stream
            << "[ExecutionPublisher] TRADE | TradeID=789 | BuyOrder=456 | "
               "SellOrder=123 | Qty=50 | Price=1.250500 | Timestamp="
            << timestamp_us << "us\n";

    ASSERT_EQ(buffer.str(), expected_output_stream.str());
}

TEST(ExecutionPublisherTests, PublishRejection)
{
    common::OrderID orderId = 999;
    common::ClientID clientId = "client-abc";
    std::string_view reason = "Insufficient funds";

    std::stringstream buffer;
    {
        CoutRedirector redirect(buffer);
        trading_core::ExecutionPublisher::publishRejection(orderId, clientId,
                                                           reason);
    }

    std::string expected_output
            = "[ExecutionPublisher] REJECT | OrderID=999 | Client=client-abc | "
              "Reason=Insufficient funds\n";
    ASSERT_EQ(buffer.str(), expected_output);
}
