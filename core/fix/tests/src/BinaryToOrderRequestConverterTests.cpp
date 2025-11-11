#include "fix/BinaryToOrderRequestConverter.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include <gtest/gtest.h>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace {
    std::vector<char> createFixMessage()
    {
        constexpr char SOH = fix::SOH;
        std::stringstream bodySs;
        bodySs << static_cast<int>(fix::Tag::ClOrdID) << "=12345" << SOH;
        bodySs << static_cast<int>(fix::Tag::Symbol) << "=EURUSD" << SOH;
        bodySs << static_cast<int>(fix::Tag::Side) << "=" << fix::ORDER_SIDE_BUY
               << SOH;
        bodySs << static_cast<int>(fix::Tag::OrdType) << "="
               << fix::ORDER_TYPE_LIMIT << SOH;
        bodySs << static_cast<int>(fix::Tag::OrderQty) << "=1000" << SOH;
        bodySs << static_cast<int>(fix::Tag::Price) << "=1.2345" << SOH;
        bodySs << static_cast<int>(fix::Tag::TimeInForce) << "="
               << fix::TIME_IN_FORCE_DAY << SOH;

        std::string body_str = bodySs.str();

        std::stringstream headerSs;
        headerSs << static_cast<int>(fix::Tag::BeginString) << "="
                 << fix::FIX_BEGIN_STRING << SOH;
        headerSs << static_cast<int>(fix::Tag::BodyLength) << "="
                 << body_str.length() << SOH;
        headerSs << static_cast<int>(fix::Tag::MsgType) << "="
                 << fix::MSG_TYPE_NEW_ORDER_SINGLE << SOH;
        headerSs << static_cast<int>(fix::Tag::SenderCompID) << "=1" << SOH;
        headerSs << static_cast<int>(fix::Tag::TargetCompID) << "=2" << SOH;
        headerSs << static_cast<int>(fix::Tag::MsgSeqNum) << "=3" << SOH;
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

        std::string finalStr = finalSs.str();
        return {finalStr.begin(), finalStr.end()};
    }
} // namespace

TEST(BinaryToOrderRequestConverterTests, BasicConversion)
{
    const std::vector<char> binaryData = createFixMessage();

    const fix::OrderRequest orderRequest
            = fix::BinaryToOrderRequestConverter::convert(binaryData);

    ASSERT_EQ(orderRequest.getSenderCompId(), 1);
    ASSERT_EQ(orderRequest.getTargetCompId(), 2);
    ASSERT_EQ(orderRequest.getMsgSeqNum(), 3);
    ASSERT_EQ(orderRequest.getClientOrderId(), 12345);
    ASSERT_EQ(orderRequest.getSymbol(), common::Instrument::EURUSD);
    ASSERT_EQ(orderRequest.getSide(), common::OrderSide::Buy);
    ASSERT_EQ(orderRequest.getType(), common::OrderType::Limit);
    ASSERT_EQ(orderRequest.getTimeInForce(), common::TimeInForce::DAY);
    ASSERT_EQ(orderRequest.getQuantity(), 1000);
    ASSERT_DOUBLE_EQ(orderRequest.getPrice(), 1.2345);
}
