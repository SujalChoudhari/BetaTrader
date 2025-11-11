#include "fix/BinaryToOrderRequestConverter.h"
#include "common/Instrument.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include <map>
#include <numeric>
#include <stdexcept>
#include <string_view>

namespace fix {

    namespace {

        std::map<int, std::string_view> splitToMap(std::string_view str,
                                                   const char delimiter)
        {
            std::map<int, std::string_view> result;
            size_t start = 0;
            size_t end = str.find(delimiter);
            while (end != std::string_view::npos) {
                std::string_view token = str.substr(start, end - start);
                size_t equalsPos = token.find('=');
                if (equalsPos != std::string_view::npos) {
                    int tag = std::stoi(
                            std::string(token.substr(0, equalsPos)));
                    result[tag] = token.substr(equalsPos + 1);
                }
                start = end + 1;
                end = str.find(delimiter, start);
            }
            return result;
        }

        common::OrderSide charToOrderSide(const char c)
        {
            if (c == ORDER_SIDE_BUY) return common::OrderSide::Buy;
            if (c == ORDER_SIDE_SELL) return common::OrderSide::Sell;
            throw std::invalid_argument("Invalid OrderSide char");
        }

        common::OrderType charToOrderType(const char c)
        {
            if (c == ORDER_TYPE_MARKET) return common::OrderType::Market;
            if (c == ORDER_TYPE_LIMIT) return common::OrderType::Limit;
            throw std::invalid_argument("Invalid OrderType char");
        }

        common::TimeInForce charToTimeInForce(const char c)
        {
            if (c == TIME_IN_FORCE_DAY) return common::TimeInForce::DAY;
            if (c == TIME_IN_FORCE_GTC) return common::TimeInForce::GTC;
            if (c == TIME_IN_FORCE_IOC) return common::TimeInForce::IOC;
            if (c == TIME_IN_FORCE_FOK) return common::TimeInForce::FOK;
            throw std::invalid_argument("Invalid TimeInForce char");
        }

    } // namespace

    OrderRequest
    BinaryToOrderRequestConverter::convert(const std::vector<char>& binaryData)
    {
        const std::string_view messageStringView(binaryData.data(),
                                                 binaryData.size());

        const std::string checksumTag
                = std::string(1, SOH)
                  + std::to_string(static_cast<int>(Tag::CheckSum)) + "=";
        const size_t checksumPos = messageStringView.rfind(checksumTag);
        if (checksumPos == std::string_view::npos) {
            throw std::runtime_error("FIX message does not contain a checksum");
        }

        const std::string_view messageUpToChecksum
                = messageStringView.substr(0, checksumPos + 1);
        const int expectedChecksum = std::stoi(std::string(
                messageStringView.substr(checksumPos + checksumTag.length())));

        unsigned int calculatedChecksum = 0;
        for (const char c: messageUpToChecksum) {
            calculatedChecksum += static_cast<unsigned int>(c);
        }
        calculatedChecksum %= 256;

        if (calculatedChecksum != expectedChecksum) {
            throw std::runtime_error("Checksum validation failed");
        }

        const auto tagMap = splitToMap(messageStringView, SOH);

        return {std::stoul(std::string(
                        tagMap.at(static_cast<int>(Tag::SenderCompID)))),
                std::stoul(std::string(
                        tagMap.at(static_cast<int>(Tag::TargetCompID)))),
                std::stoull(std::string(
                        tagMap.at(static_cast<int>(Tag::MsgSeqNum)))),
                std::stoull(
                        std::string(tagMap.at(static_cast<int>(Tag::ClOrdID)))),
                common::from_string(tagMap.at(static_cast<int>(Tag::Symbol))),
                charToOrderSide(tagMap.at(static_cast<int>(Tag::Side)).front()),
                charToOrderType(
                        tagMap.at(static_cast<int>(Tag::OrdType)).front()),
                charToTimeInForce(
                        tagMap.at(static_cast<int>(Tag::TimeInForce)).front()),
                std::stoull(std::string(
                        tagMap.at(static_cast<int>(Tag::OrderQty)))),
                std::stod(std::string(tagMap.at(static_cast<int>(Tag::Price)))),
                tagMap.contains(static_cast<int>(Tag::StopPrice))
                        ? std::stod(std::string(
                                  tagMap.at(static_cast<int>(Tag::StopPrice))))
                        : 0.0,
                static_cast<uint8_t>(
                        tagMap.contains(static_cast<int>(Tag::SettlType))
                                ? std::stoi(std::string(tagMap.at(
                                          static_cast<int>(Tag::SettlType))))
                                : 0),
                tagMap.contains(static_cast<int>(Tag::SettlDate))
                        ? std::stoul(std::string(
                                  tagMap.at(static_cast<int>(Tag::SettlDate))))
                        : 0};
    }

} // namespace fix
