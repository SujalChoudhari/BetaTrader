#include "common_fix/FixUtils.h"
#include "common_fix/Protocol.h"
#include <gtest/gtest.h>
#include <chrono>
#include <ctime>

using namespace fix;

TEST(FixUtilsTests, SplitToMap) {
    std::string_view msg = "8=FIX.4.4\x01" "9=100\x01" "INVALID_TOKEN\x01" "35=8\x01";
    auto result = splitToMap(msg, '\x01');
    
    EXPECT_EQ(result[8], "FIX.4.4");
    EXPECT_EQ(result[9], "100");
    EXPECT_EQ(result[35], "8");
    EXPECT_EQ(result.size(), 3);
}

TEST(FixUtilsTests, OrderSideConversion) {
    EXPECT_EQ(charToOrderSide(ORDER_SIDE_BUY), common::OrderSide::Buy);
    EXPECT_EQ(charToOrderSide(ORDER_SIDE_SELL), common::OrderSide::Sell);
    EXPECT_THROW(charToOrderSide('X'), std::invalid_argument);
}

TEST(FixUtilsTests, OrderTypeConversion) {
    EXPECT_EQ(charToOrderType(ORDER_TYPE_MARKET), common::OrderType::Market);
    EXPECT_EQ(charToOrderType(ORDER_TYPE_LIMIT), common::OrderType::Limit);
    EXPECT_THROW(charToOrderType('X'), std::invalid_argument);
}

static std::chrono::system_clock::time_point makeExpected(
    int year, int mon, int day,
    int hour, int min, int sec,
    int millis)
{
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon  = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min  = min;
    tm.tm_sec  = sec;

    auto tt = std::mktime(&tm);
    auto tp = std::chrono::system_clock::from_time_t(tt);
    tp += std::chrono::milliseconds(millis);

    return tp;
}

TEST(ParseTimestampTest, ParsesValidTimestamp)
{
    std::string ts = "20250313-14:25:10.123";

    auto parsed = parseTimestamp(ts);
    auto expected = makeExpected(2025,3,13,14,25,10,123);

    EXPECT_EQ(parsed, expected);
}

TEST(ParseTimestampTest, ParsesWithoutMilliseconds)
{
    std::string ts = "20250313-14:25:10";

    auto parsed = parseTimestamp(ts);
    auto expected = makeExpected(2025,3,13,14,25,10,0);

    EXPECT_EQ(parsed, expected);
}

TEST(ParseTimestampTest, InvalidFormatThrows)
{
    std::string ts = "invalid";

    EXPECT_THROW(parseTimestamp(ts), std::runtime_error);
}

TEST(ParseTimestampTest, DifferentMilliseconds)
{
    std::string ts = "20240101-00:00:00.999";

    auto parsed = parseTimestamp(ts);
    auto expected = makeExpected(2024,1,1,0,0,0,999);

    EXPECT_EQ(parsed, expected);
}