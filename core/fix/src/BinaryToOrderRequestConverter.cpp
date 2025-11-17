#include "fix/BinaryToOrderRequestConverter.h"
#include "common/Instrument.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include "logging/Logger.h"
#include <map>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace fix {

    // TODO: Consider moving this helper function to a common utility or making it a private static member if only used by this class.
    std::map<int, std::string_view> splitToMap(std::string_view str,
                                               const char delimiter)
    {
        std::map<int, std::string_view> result;
        size_t start = 0;
        size_t end = str.find(delimiter);
        while (end != std::string_view::npos) {
            std::string_view token = str.substr(start, end - start);
            if (size_t equalsPos = token.find('=');
                equalsPos != std::string_view::npos) {
                int tag = std::stoi(std::string(token.substr(0, equalsPos)));
                result[tag] = token.substr(equalsPos + 1);
            }
            start = end + 1;
            end = str.find(delimiter, start);
        }
        return result;
    }

    // TODO: Consider moving this helper function to a common utility or making it a private static member if only used by this class.
    common::OrderSide charToOrderSide(const char c)
    {
        if (c == fix::ORDER_SIDE_BUY) return common::OrderSide::Buy;
        if (c == fix::ORDER_SIDE_SELL) return common::OrderSide::Sell;
        throw std::invalid_argument("Invalid OrderSide char");
    }

    std::optional<OrderRequest>
    BinaryToOrderRequestConverter::convert(const std::string& fixMessage)
    {
        try {
            const std::string_view messageStringView(fixMessage);

            const std::string checksumTag
                    = std::string(1, fix::SOH)
                      + std::to_string(static_cast<int>(fix::Tag::CheckSum)) + "=";
            const size_t checksumPos = messageStringView.rfind(checksumTag);
            if (checksumPos == std::string_view::npos) {
                LOG_WARN("BinaryToOrderRequestConverter::convert - FIX message does not contain a checksum: {}", fixMessage);
                return std::nullopt;
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
                LOG_WARN("BinaryToOrderRequestConverter::convert - Checksum validation failed. Expected: {}, Calculated: {}. Message: {}",
                         expectedChecksum, calculatedChecksum, fixMessage);
                return std::nullopt;
            }

            const auto tagMap = splitToMap(messageStringView, fix::SOH);

            OrderRequest request = {
                std::string(tagMap.at(static_cast<int>(fix::Tag::SenderCompID))),
                std::stoull(std::string(tagMap.at(static_cast<int>(fix::Tag::ClOrdID)))),
                common::from_string(tagMap.at(static_cast<int>(fix::Tag::Symbol))),
                charToOrderSide(tagMap.at(static_cast<int>(fix::Tag::Side)).front()),
                std::stoull(std::string(tagMap.at(static_cast<int>(fix::Tag::OrderQty)))),
                std::stod(std::string(tagMap.at(static_cast<int>(fix::Tag::Price))))
            };
            return request;
        } catch (const std::exception& e) {
            LOG_ERROR("BinaryToOrderRequestConverter::convert - Parsing failed for message '{}': {}", fixMessage, e.what());
            return std::nullopt;
        }
    }

} // namespace fix
