#include "common_fix/OutboundMessageBuilder.h"
#include "common_fix/Protocol.h"
#include "fix/FixMessageFramer.h"
#include <gtest/gtest.h>

namespace {
    std::string tamperChecksum(std::string message)
    {
        const auto checksumPos = message.rfind("10=");
        EXPECT_NE(checksumPos, std::string::npos);
        if (checksumPos != std::string::npos) {
            message[checksumPos + 3]
                    = message[checksumPos + 3] == '0' ? '1' : '0';
        }
        return message;
    }
} // namespace

TEST(FixMessageFramerTests, UsesBodyLengthWhenPayloadContainsChecksumLikeField)
{
    const auto message = fix::OutboundMessageBuilder::buildMessage(
            "CLIENT_A", "BETA_EXCHANGE", 1, "D",
            "11=12345\x01"
            "999=payload\x01"
            "10=111\x01"
            "55=EURUSD\x01");

    const auto result = fix::extractNextFrame(message);

    ASSERT_EQ(result.status, fix::FrameStatus::Complete);
    EXPECT_EQ(result.frame, message);
    EXPECT_EQ(result.consumed, message.size());
}

TEST(FixMessageFramerTests, WaitsForCompleteBodyAndChecksum)
{
    const auto message = fix::OutboundMessageBuilder::buildLogon(
            "CLIENT_A", "BETA_EXCHANGE", 1, 30);

    const auto result
            = fix::extractNextFrame(message.substr(0, message.size() - 1));

    EXPECT_EQ(result.status, fix::FrameStatus::NeedMoreData);
}

TEST(FixMessageFramerTests, RejectsIncorrectBodyLength)
{
    const auto message = fix::OutboundMessageBuilder::buildLogon(
            "CLIENT_A", "BETA_EXCHANGE", 1, 30);
    const auto bodyLengthPos = message.find("9=");
    const auto bodyLengthEnd = message.find(fix::SOH, bodyLengthPos);
    ASSERT_NE(bodyLengthPos, std::string::npos);
    ASSERT_NE(bodyLengthEnd, std::string::npos);

    const auto bodyLength = std::stoul(message.substr(
            bodyLengthPos + 2, bodyLengthEnd - bodyLengthPos - 2));
    auto malformed = message;
    malformed.replace(bodyLengthPos + 2, bodyLengthEnd - bodyLengthPos - 2,
                      std::to_string(bodyLength + 1));

    const auto result = fix::extractNextFrame(malformed);

    EXPECT_EQ(result.status, fix::FrameStatus::Malformed);
}

TEST(FixMessageFramerTests, RejectsChecksumFailure)
{
    const auto message = fix::OutboundMessageBuilder::buildLogon(
            "CLIENT_A", "BETA_EXCHANGE", 1, 30);

    const auto result = fix::extractNextFrame(tamperChecksum(message));

    EXPECT_EQ(result.status, fix::FrameStatus::Malformed);
}

TEST(FixMessageFramerTests, RejectsMalformedPrefix)
{
    const auto result = fix::extractNextFrame("not-a-fix-message");

    EXPECT_EQ(result.status, fix::FrameStatus::Malformed);
}
