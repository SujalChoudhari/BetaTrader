#include "fix/BinaryToOrderRequestConverter.h"
#include "common/Instrument.h"
#include "common_fix/FixUtils.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "logging/Logger.h"
#include <map>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace fix {

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

            auto getTagSafe = [&](fix::Tag tag, const std::string& defaultValue = "") -> std::string {
                auto it = tagMap.find(static_cast<int>(tag));
                return (it != tagMap.end()) ? std::string(it->second) : defaultValue;
            };

            OrderRequest request = {
                std::string(tagMap.at(static_cast<int>(fix::Tag::SenderCompID))),
                std::stoull(std::string(tagMap.at(static_cast<int>(fix::Tag::ClOrdID)))),
                common::from_string(tagMap.at(static_cast<int>(fix::Tag::Symbol))),
                charToOrderSide(tagMap.at(static_cast<int>(fix::Tag::Side)).front()),
                charToOrderType(getTagSafe(fix::Tag::OrdType, "2").front()), // Default to Limit (2)
                std::stoull(std::string(tagMap.at(static_cast<int>(fix::Tag::OrderQty)))),
                std::stod(getTagSafe(fix::Tag::Price, "0.0"))
            };
            return request;
        } catch (const std::exception& e) {
            LOG_ERROR("BinaryToOrderRequestConverter::convert - Parsing failed for message '{}': {}", fixMessage, e.what());
            return std::nullopt;
        }
    }

} // namespace fix
