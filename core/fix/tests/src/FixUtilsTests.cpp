#include "fix/FixUtils.h"
#include "common_fix/Protocol.h"
#include <gtest/gtest.h>

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
