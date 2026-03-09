#include "fix/BinaryToOrderRequestConverter.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include <gtest/gtest.h>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace {
    std::string createFixMessageString() // Changed return type to std::string
    {
        constexpr char SOH = fix::SOH;
        std::stringstream bodySs;
        bodySs << static_cast<int>(fix::Tag::ClOrdID) << "=12345" << SOH;
        bodySs << static_cast<int>(fix::Tag::Symbol) << "=EURUSD" << SOH;
        bodySs << static_cast<int>(fix::Tag::Side) << "=" << fix::ORDER_SIDE_BUY
               << SOH;
        bodySs << static_cast<int>(fix::Tag::OrderQty) << "=1000" << SOH;
        bodySs << static_cast<int>(fix::Tag::Price) << "=1.2345" << SOH;

        std::string body_str = bodySs.str();

        std::stringstream headerSs;
        headerSs << static_cast<int>(fix::Tag::BeginString) << "="
                 << fix::FIX_BEGIN_STRING << SOH;
        headerSs << static_cast<int>(fix::Tag::BodyLength) << "="
                 << body_str.length() << SOH;
        headerSs << static_cast<int>(fix::Tag::MsgType) << "="
                 << fix::MSG_TYPE_NEW_ORDER_SINGLE << SOH;
        headerSs << static_cast<int>(fix::Tag::SenderCompID) << "=CLIENT_A" << SOH;
        headerSs << static_cast<int>(fix::Tag::TargetCompID) << "=SERVER_B" << SOH;
        headerSs << static_cast<int>(fix::Tag::MsgSeqNum) << "=1" << SOH;
        headerSs << static_cast<int>(fix::Tag::SendingTime)
                 << "=20230401-12:30:00.000" << SOH;

        std::string headerStr = headerSs.str();
        std::string messageSansChecksum = headerStr + body_str;

        unsigned int checksum = 0;
        for (const char c: messageSansChecksum) {
            checksum += static_cast<unsigned int>(c);
        }
        checksum %= 256;

        std::stringstream finalSs;
        finalSs << messageSansChecksum;
        finalSs << static_cast<int>(fix::Tag::CheckSum) << "="
                << std::setfill('0') << std::setw(3) << checksum << SOH;

        return finalSs.str(); // Return std::string
    }
} // namespace

TEST(BinaryToOrderRequestConverterTests, BasicConversion)
{
    const std::string fixMessage = createFixMessageString(); // Get std::string

    const std::optional<fix::OrderRequest> orderRequestOpt // Changed type to optional
            = fix::BinaryToOrderRequestConverter::convert(fixMessage);

    ASSERT_TRUE(orderRequestOpt.has_value()); // Assert that optional has a value
    const fix::OrderRequest& orderRequest = orderRequestOpt.value(); // Get the underlying value

    ASSERT_EQ(orderRequest.senderCompID, "CLIENT_A");
    ASSERT_EQ(orderRequest.clientOrderId, 12345);
    ASSERT_EQ(orderRequest.symbol, common::Instrument::EURUSD);
    ASSERT_EQ(orderRequest.side, common::OrderSide::Buy);
    ASSERT_EQ(orderRequest.quantity, 1000);
    ASSERT_DOUBLE_EQ(orderRequest.price, 1.2345);
}

TEST(BinaryToOrderRequestConverterTests, MissingChecksum)
{
    std::string msg = "8=FIX.4.4\x01" "9=50\x01" "35=D\x01" "49=CLIENT_A\x01";
    auto result = fix::BinaryToOrderRequestConverter::convert(msg);
    EXPECT_FALSE(result.has_value());
}

TEST(BinaryToOrderRequestConverterTests, ChecksumValidationFailed)
{
    std::string msg = createFixMessageString();
    // Corrupt the checksum
    size_t pos = msg.rfind("10=");
    msg[pos + 3] = '9'; 
    msg[pos + 4] = '9';
    msg[pos + 5] = '9';
    
    auto result = fix::BinaryToOrderRequestConverter::convert(msg);
    EXPECT_FALSE(result.has_value());
}

TEST(BinaryToOrderRequestConverterTests, InvalidChecksumFormat)
{
    std::string msg = createFixMessageString();
    size_t pos = msg.rfind("10=");
    msg.replace(pos + 3, 3, "ABC");
    
    auto result = fix::BinaryToOrderRequestConverter::convert(msg);
    EXPECT_FALSE(result.has_value());
}

TEST(BinaryToOrderRequestConverterTests, MissingMandatoryTags)
{
    // Missing Side (Tag 54)
    std::string msg = "8=FIX.4.4\x01" "9=40\x01" "35=D\x01" "49=CLIENT_A\x01" "11=1\x01" "55=EURUSD\x01" "38=100\x01" "44=1.1\x01" "10=185\x01";
    auto result = fix::BinaryToOrderRequestConverter::convert(msg);
    EXPECT_FALSE(result.has_value());
}

TEST(BinaryToOrderRequestConverterTests, InvalidQuantityFormat)
{
    // Tag 38 is "ABC" instead of numeric
    std::string msg = "8=FIX.4.4\x01" "9=45\x01" "35=D\x01" "49=CLIENT_A\x01" "11=1\x01" "55=EURUSD\x01" "54=1\x01" "38=ABC\x01" "44=1.1\x01" "10=050\x01";
    auto result = fix::BinaryToOrderRequestConverter::convert(msg);
    EXPECT_FALSE(result.has_value());
}

TEST(BinaryToOrderRequestConverterTests, InvalidSymbol)
{
    // Invalid symbol "INVALID"
    std::string msg = "8=FIX.4.4\x01" "9=45\x01" "35=D\x01" "49=CLIENT_A\x01" "11=1\x01" "55=INVALID\x01" "54=1\x01" "38=100\x01" "44=1.1\x01" "10=150\x01";
    auto result = fix::BinaryToOrderRequestConverter::convert(msg);
    EXPECT_FALSE(result.has_value());
}

