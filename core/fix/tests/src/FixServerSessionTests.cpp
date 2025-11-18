#include "fix/FixServer.h"
#include "trading_core/ExecutionPublisher.h"
#include "trading_core/TradingCore.h"
#include <future>
#include <gtest/gtest.h>

class MockTradingCore : public trading_core::TradingCore {
public:
    MockTradingCore() : trading_core::TradingCore(nullptr, false) {}

    // Override to capture submitted commands for inspection
    void submitCommand(std::unique_ptr<trading_core::Command> command) const override {
        last_command_ = std::move(command);
    }

    mutable std::unique_ptr<trading_core::Command> last_command_;
};

TEST(FixServerSessionTests, NewOrderEndToEnd) {
    // This test verifies the integration between the TradingCore's publisher
    // and a subscriber (like the FixServer). It ensures that when the core
    // publishes an event, the subscriber receives a correctly formed ExecutionReport.
    
    // 1. Setup
    MockTradingCore mock_core;
    
    // We use a promise/future to wait for the asynchronous callback.
    // The promise will be fulfilled with the ExecutionReport from the publisher.
    std::promise<fix::ExecutionReport> promise;
    auto future = promise.get_future();

    // 2. Subscribe to the TradingCore's execution report stream.
    // The callback's job is to fulfill the promise.
    mock_core.subscribeToExecutions([&](const fix::ExecutionReport& report) {
        promise.set_value(report);
    });

    // 3. Manually create a test order and publish it using the real ExecutionPublisher.
    // This simulates what the TradingCore's WorkerThread would do.
    // The ClientID is a string representation of the session ID.
    common::Order test_order(
        123, // Order ID
        common::Instrument::EURUSD, 
        "456", // ClientID (Session ID)
        common::OrderSide::Buy, 
        common::OrderType::Limit, 
        common::TimeInForce::DAY, 
        1000, // Quantity
        1.25, // Price
        std::chrono::system_clock::now()
    );
    trading_core::ExecutionPublisher::publishExecution(test_order, "NEW");

    // 4. Wait for the callback to be invoked.
    auto result = future.wait_for(std::chrono::seconds(2));
    ASSERT_EQ(result, std::future_status::ready) << "Test timed out. The subscriber callback was never called.";
    
    // 5. Assertions
    // Verify that the received ExecutionReport contains the correct data.
    auto received_report = future.get();
    ASSERT_EQ(received_report.getExchangeOrderId(), 123);
    ASSERT_EQ(received_report.getTargetCompId(), 456); // Should match the ClientID/SessionID
    ASSERT_EQ(received_report.getStatus(), common::OrderStatus::New);
    ASSERT_EQ(received_report.getSymbol(), common::Instrument::EURUSD);
    ASSERT_EQ(received_report.getOrderQuantity(), 1000);
    ASSERT_EQ(received_report.getLeavesQuantity(), 1000);
}
