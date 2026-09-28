#include "fix/BinaryToCancelOrderRequestConverter.h"
#include "fix/BinaryToModifyOrderRequestConverter.h"
#include <gtest/gtest.h>

TEST(ClientOrderIdLifecycleTests, ParsesNonNumericModifyClientOrderIds)
{
    const std::string message = "11=modify-1\x01"
                                "41=client-order-abc\x01"
                                "55=EURUSD\x01"
                                "54=1\x01"
                                "38=100\x01"
                                "40=2\x01"
                                "44=1.25\x01"
                                "60=20260928-10:00:00.000\x01";

    const auto result
            = fix::BinaryToModifyOrderRequestConverter::convert(message);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->clOrdID, "modify-1");
    EXPECT_EQ(result->origClOrdID, "client-order-abc");
    EXPECT_EQ(result->orderID, 0);
}

TEST(ClientOrderIdLifecycleTests, ParsesNonNumericCancelClientOrderIds)
{
    const std::string message = "11=cancel-1\x01"
                                "41=client-order-abc\x01"
                                "55=EURUSD\x01"
                                "54=1\x01"
                                "60=20260928-10:00:00.000\x01";

    const auto result
            = fix::BinaryToCancelOrderRequestConverter::convert(message);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->clOrdID, "cancel-1");
    EXPECT_EQ(result->origClOrdID, "client-order-abc");
    EXPECT_EQ(result->orderID, 0);
}
