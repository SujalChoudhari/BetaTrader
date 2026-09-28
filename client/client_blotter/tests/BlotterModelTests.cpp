#include "blotter/BlotterModel.h"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

namespace {

    fix::ExecutionReport
    report(const std::string& clientOrderId, const std::string& executionId,
           common::OrderStatus status, const std::string& text,
           common::Quantity cumulativeQuantity, common::Quantity leavesQuantity,
           common::Price lastPrice, common::Quantity lastQuantity,
           common::Timestamp timestamp)
    {
        return fix::ExecutionReport(1, 2, 1, 42, clientOrderId, executionId,
                                    status, text, common::Instrument::EURUSD,
                                    common::OrderSide::Buy, 100,
                                    cumulativeQuantity, leavesQuantity,
                                    lastPrice, lastQuantity, timestamp);
    }

} // namespace

TEST(BlotterModelTests, TracksLifecycleAndSeparatesOpenOrdersFromHistory)
{
    client_blotter::BlotterModel model;
    const auto t0
            = std::chrono::system_clock::time_point(std::chrono::seconds(1));
    model.recordOrderIntent({"order-1", common::Instrument::EURUSD,
                             common::OrderSide::Buy, common::OrderType::Limit,
                             1.2345, 100});

    model.processExecution(report("order-1", "exec-new",
                                  common::OrderStatus::New, "NEW", 0, 100,
                                  1.2345, 0, t0));
    auto open = model.activeOrders();
    ASSERT_EQ(open.size(), 1U);
    EXPECT_EQ(open.front().clientOrderId, "order-1");
    EXPECT_EQ(open.front().orderType, common::OrderType::Limit);
    EXPECT_EQ(open.front().originalQuantity, 100U);
    EXPECT_EQ(open.front().remainingQuantity, 100U);

    model.processExecution(report(
            "order-1", "exec-partial", common::OrderStatus::PartiallyFilled,
            "TRADE", 40, 60, 1.2346, 40, t0 + std::chrono::seconds(1)));
    open = model.activeOrders();
    ASSERT_EQ(open.size(), 1U);
    EXPECT_EQ(open.front().remainingQuantity, 60U);

    model.processExecution(report("order-1", "exec-fill",
                                  common::OrderStatus::Filled, "TRADE", 100, 0,
                                  1.2347, 60, t0 + std::chrono::seconds(2)));
    EXPECT_TRUE(model.activeOrders().empty());

    const auto history = model.history();
    ASSERT_EQ(history.size(), 3U);
    EXPECT_EQ(history[1].lastQuantity, 40U);
    EXPECT_EQ(history[1].exchangeOrderId, 42U);
    EXPECT_EQ(history[2].lastPrice, 1.2347);
}

TEST(BlotterModelTests,
     RetainsCancellationAndRejectionReasonsAndDeduplicatesReports)
{
    client_blotter::BlotterModel model;
    model.recordOrderIntent({"cancel-me", common::Instrument::USDJPY,
                             common::OrderSide::Sell, common::OrderType::Market,
                             0.0, 10});
    const auto timestamp
            = std::chrono::system_clock::time_point(std::chrono::seconds(5));

    model.processExecution(report("cancel-me", "exec-cancel",
                                  common::OrderStatus::New, "NEW", 0, 10, 1.0,
                                  0, timestamp));
    const auto cancelled
            = report("cancel-me", "exec-cancel", common::OrderStatus::Cancelled,
                     "User cancelled", 0, 10, 0.0, 0, timestamp);
    model.processExecution(cancelled);
    model.processExecution(cancelled);

    model.processExecution(report("reject-me", "exec-reject",
                                  common::OrderStatus::Rejected, "Risk limit",
                                  0, 0, 0.0, 0, timestamp));

    const auto history = model.history();
    ASSERT_EQ(history.size(), 3U);
    EXPECT_EQ(history[1].reason, "User cancelled");
    EXPECT_EQ(history[2].reason, "Risk limit");
    EXPECT_TRUE(model.activeOrders().empty());
}

TEST(BlotterModelTests, SnapshotsRemainSafeAcrossConcurrentUpdates)
{
    client_blotter::BlotterModel model;
    model.recordOrderIntent({"thread-safe", common::Instrument::EURUSD,
                             common::OrderSide::Buy, common::OrderType::Limit,
                             1.0, 1});

    std::thread producer([&model]() {
        const auto timestamp = std::chrono::system_clock::now();
        for (int i = 0; i < 100; ++i) {
            model.processExecution(report("thread-safe",
                                          "exec-" + std::to_string(i),
                                          common::OrderStatus::PartiallyFilled,
                                          "TRADE", i, 1, 1.0, 0, timestamp));
        }
    });

    for (int i = 0; i < 100; ++i) {
        (void)model.activeOrders();
        (void)model.history();
    }
    producer.join();

    EXPECT_EQ(model.history().size(), 100U);
}
