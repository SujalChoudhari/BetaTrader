#include "fix/BinaryToMarketDataRequestConverter.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include "common/Types.h"
#include "common/Instrument.h"
#include "logging/Logger.h"
#include <string_view>
#include <charconv>
#include <map>

namespace fix
{

// TODO: Implement a robust FIX message parsing utility (e.g., using a tag-value map approach similar to BinaryToOrderRequestConverter)
// The current implementation is a simplified skeleton and needs to be made production-ready.

std::optional<MarketDataRequest> BinaryToMarketDataRequestConverter::convert(const std::string& fixMessage)
{
    MarketDataRequest mdRequest;
    try {
        size_t pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::MDReqID)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                mdRequest.mdReqID = fixMessage.substr(pos + std::to_string(static_cast<int>(fix::Tag::MDReqID)).length() + 1, endPos - (pos + std::to_string(static_cast<int>(fix::Tag::MDReqID)).length() + 1));
            }
        }

        pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::SubscriptionRequestType)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                mdRequest.subscriptionRequestType = fixMessage[pos + std::to_string(static_cast<int>(fix::Tag::SubscriptionRequestType)).length() + 1];
            }
        }

        pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::MarketDepth)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                std::string depthStr = fixMessage.substr(pos + std::to_string(static_cast<int>(fix::Tag::MarketDepth)).length() + 1, endPos - (pos + std::to_string(static_cast<int>(fix::Tag::MarketDepth)).length() + 1));
                int depth;
                auto [ptr, ec] = std::from_chars(depthStr.data(), depthStr.data() + depthStr.size(), depth);
                if (ec == std::errc()) {
                    mdRequest.marketDepth = depth;
                } else {
                    LOG_WARN("BinaryToMarketDataRequestConverter::convert - Invalid MarketDepth: {} in message: {}", depthStr, fixMessage);
                    return std::nullopt;
                }
            }
        }

        pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::NoRelatedSym)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                std::string numSymbolsStr = fixMessage.substr(pos + std::to_string(static_cast<int>(fix::Tag::NoRelatedSym)).length() + 1, endPos - (pos + std::to_string(static_cast<int>(fix::Tag::NoRelatedSym)).length() + 1));
                int numSymbols;
                auto [ptr, ec] = std::from_chars(numSymbolsStr.data(), numSymbolsStr.data() + numSymbolsStr.size(), numSymbols);
                if (ec == std::errc()) {
                    size_t currentSearchPos = endPos + 1;
                    for (int i = 0; i < numSymbols; ++i) {
                        size_t symbolPos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::Symbol)) + "=", currentSearchPos);
                        if (symbolPos != std::string::npos) {
                            size_t symbolEndPos = fixMessage.find(fix::SOH, symbolPos);
                            if (symbolEndPos != std::string::npos) {
                                std::string symbolStr = fixMessage.substr(symbolPos + std::to_string(static_cast<int>(fix::Tag::Symbol)).length() + 1, symbolEndPos - (symbolPos + std::to_string(static_cast<int>(fix::Tag::Symbol)).length() + 1));
                                mdRequest.symbols.push_back(common::from_string(symbolStr));
                                currentSearchPos = symbolEndPos + 1;
                            } else {
                                LOG_WARN("BinaryToMarketDataRequestConverter::convert - Malformed Symbol tag in message: {}", fixMessage);
                                return std::nullopt;
                            }
                        } else {
                            LOG_WARN("BinaryToMarketDataRequestConverter::convert - Missing Symbol tag in repeating group for message: {}", fixMessage);
                            return std::nullopt;
                        }
                    }
                } else {
                    LOG_WARN("BinaryToMarketDataRequestConverter::convert - Invalid NoRelatedSym: {} in message: {}", numSymbolsStr, fixMessage);
                    return std::nullopt;
                }
            }
        }

        LOG_INFO("BinaryToMarketDataRequestConverter::convert - Parsed MarketDataRequest (skeleton). MDReqID: {}", mdRequest.mdReqID);
        return mdRequest;
    } catch (const std::exception& e) {
        LOG_ERROR("BinaryToMarketDataRequestConverter::convert - Exception during parsing for message '{}': {}", fixMessage, e.what());
        return std::nullopt;
    }
}

} // namespace fix
