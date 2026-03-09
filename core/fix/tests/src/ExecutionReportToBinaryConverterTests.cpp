#include "fix/ExecutionReportToBinaryConverter.h"
#include "common_fix/Protocol.h"
#include <gtest/gtest.h>
#include <string>

// Helper to find a tag's value in a FIX message
std::string get_tag_value(const std::string& msg, int tag) {
    std::string tag_str = std::to_string(tag) + "=";
    size_t start_pos = msg.find(tag_str);
    if (start_pos == std::string::npos) {
        return "";
    }
    start_pos += tag_str.length();
    size_t end_pos = msg.find(fix::SOH, start_pos);
    if (end_pos == std::string::npos) {
        return "";
    }
    return msg.substr(start_pos, end_pos - start_pos);
}

TEST(ExecutionReportToBinaryConverterTests, BasicConversion)
{
    fix::ExecutionReport report(
            static_cast<fix::CompID>(123), // senderCompId
            static_cast<fix::CompID>(456), // targetCompId
            static_cast<fix::SequenceNumber>(1), // msgSeqNum
            static_cast<fix::ExchangeOrderID>(999), // exchangeOrderId
            static_cast<fix::ClientOrderID>(789), // clientOrderId
            "EXEC1", // executionId
            common::OrderStatus::New,
            "New order",
            common::Instrument::EURUSD,
            common::OrderSide::Buy,
            static_cast<fix::Quantity>(1000), // orderQty
            static_cast<fix::Quantity>(0), // cumQty
            static_cast<fix::Quantity>(1000), // leavesQty
            static_cast<fix::Price>(0.0), // lastPrice
            static_cast<fix::Quantity>(0), // lastQty
            std::chrono::system_clock::now()
    );

    std::string message = fix::ExecutionReportToBinaryConverter::convert(report); // Changed to std::string

    // Basic assertions
    ASSERT_NE(message.find("35=8"), std::string::npos); // MsgType = ExecutionReport
    ASSERT_EQ(get_tag_value(message, 37), "999"); // OrderID
    ASSERT_EQ(get_tag_value(message, 11), "789"); // ClOrdID
    ASSERT_EQ(get_tag_value(message, 39), "0"); // OrdStatus = New
    ASSERT_EQ(get_tag_value(message, 55), "EURUSD"); // Symbol
    ASSERT_EQ(get_tag_value(message, 54), "1"); // Side = Buy
    ASSERT_EQ(get_tag_value(message, 38), "1000"); // OrderQty
}

TEST(ExecutionReportToBinaryConverterTests, AllStatuses)
{
    common::OrderStatus statuses[] = {
        common::OrderStatus::PartiallyFilled,
        common::OrderStatus::Filled,
        common::OrderStatus::Cancelled,
        common::OrderStatus::Rejected
    };
    std::string expected[] = {"1", "2", "4", "8"};

    for (int i = 0; i < 4; ++i) {
        fix::ExecutionReport report(1, 2, 3, 4, 5, "E", statuses[i], "", common::Instrument::USDINR, common::OrderSide::Sell, 100, 0, 100, 0, 0, std::chrono::system_clock::now());
        std::string msg = fix::ExecutionReportToBinaryConverter::convert(report);
        EXPECT_EQ(get_tag_value(msg, 39), expected[i]);
    }
}

TEST(ExecutionReportToBinaryConverterTests, EmptyText)
{
    fix::ExecutionReport report(1, 2, 3, 4, 5, "E", common::OrderStatus::New, "", common::Instrument::USDINR, common::OrderSide::Sell, 100, 0, 100, 0, 0, std::chrono::system_clock::now());
    std::string msg = fix::ExecutionReportToBinaryConverter::convert(report);
    EXPECT_EQ(get_tag_value(msg, 58), ""); // Tag 58 (Text) should be missing
}

TEST(ExecutionReportToBinaryConverterTests, InvalidStatusThrows)
{
    fix::ExecutionReport report(1, 2, 3, 4, 5, "E", static_cast<common::OrderStatus>(99), "", common::Instrument::USDINR, common::OrderSide::Sell, 100, 0, 100, 0, 0, std::chrono::system_clock::now());
    EXPECT_THROW(fix::ExecutionReportToBinaryConverter::convert(report), std::invalid_argument);
}

TEST(ExecutionReportToBinaryConverterTests, InvalidSideThrows)
{
    fix::ExecutionReport report(1, 2, 3, 4, 5, "E", common::OrderStatus::New, "", common::Instrument::USDINR, static_cast<common::OrderSide>(99), 100, 0, 100, 0, 0, std::chrono::system_clock::now());
    EXPECT_THROW(fix::ExecutionReportToBinaryConverter::convert(report), std::invalid_argument);
}

