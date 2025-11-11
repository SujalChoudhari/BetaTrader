#include "fix/OrderResponseToBinaryConverter.h"
#include <gtest/gtest.h>

TEST(OrderResponseToBinaryConverterTests, BasicConversion)
{
    fix::OrderResponseToBinaryConverter converter;
    fix::OrderResponse orderResponse(
            1, // senderCompId
            2, // targetCompId
            3, // msgSeqNum
            4, // exchangeOrderId
            5, // clientOrderId
            "exec_id", // executionId
            common::OrderStatus::New,
            "text", // text
            common::Instrument::EURUSD, // symbol
            common::OrderSide::Buy,
            100, // orderQty
            50, // cumQty
            50, // leavesQty
            1.2345, // lastPrice
            50, // lastQty
            std::chrono::system_clock::now() // transactTime
    );
    std::vector<char> binaryData = converter.convert(orderResponse);
    // TODO: Add assertions
    ASSERT_TRUE(true);
}
