#include "fix/BinaryToCancelOrderRequestConverter.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include "common/Types.h"
#include "common/Instrument.h"
#include "logging/Logger.h"
#include <string_view>
#include <chrono>
#include <stdexcept>

namespace fix
{

// TODO: Implement a robust FIX message parsing utility (e.g., using a tag-value map approach similar to BinaryToOrderRequestConverter)
// The current implementation is a simplified skeleton and needs to be made production-ready.

std::optional<CancelOrder> BinaryToCancelOrderRequestConverter::convert(const std::string& fixMessage)
{
    CancelOrder cancelOrder;
    try {
        size_t pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::ClOrdID)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                cancelOrder.clOrdID = fixMessage.substr(pos + std::to_string(static_cast<int>(fix::Tag::ClOrdID)).length() + 1, endPos - (pos + std::to_string(static_cast<int>(fix::Tag::ClOrdID)).length() + 1));
            } else {
                LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Malformed ClOrdID tag in message: {}", fixMessage);
                return std::nullopt;
            }
        } else {
            LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Missing ClOrdID tag in message: {}", fixMessage);
            // TODO: Decide if ClOrdID is mandatory for CancelOrder. If so, return nullopt.
        }

        pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::OrderID)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                cancelOrder.orderID = fixMessage.substr(pos + std::to_string(static_cast<int>(fix::Tag::OrderID)).length() + 1, endPos - (pos + std::to_string(static_cast<int>(fix::Tag::OrderID)).length() + 1));
            } else {
                LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Malformed OrderID tag in message: {}", fixMessage);
                return std::nullopt;
            }
        } else {
            LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Missing OrderID tag in message: {}", fixMessage);
            // TODO: Decide if OrderID is mandatory for CancelOrder. If so, return nullopt.
        }

        pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::Symbol)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                std::string symbolStr = fixMessage.substr(pos + std::to_string(static_cast<int>(fix::Tag::Symbol)).length() + 1, endPos - (pos + std::to_string(static_cast<int>(fix::Tag::Symbol)).length() + 1));
                cancelOrder.symbol = common::from_string(symbolStr);
            } else {
                LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Malformed Symbol tag in message: {}", fixMessage);
                return std::nullopt;
            }
        } else {
            LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Missing Symbol tag in message: {}", fixMessage);
            // TODO: Decide if Symbol is mandatory for CancelOrder. If so, return nullopt.
        }

        pos = fixMessage.find(std::to_string(static_cast<int>(fix::Tag::Side)) + "=");
        if (pos != std::string::npos) {
            size_t endPos = fixMessage.find(fix::SOH, pos);
            if (endPos != std::string::npos) {
                char sideChar = fixMessage[pos + std::to_string(static_cast<int>(fix::Tag::Side)).length() + 1];
                if (sideChar == fix::ORDER_SIDE_BUY) {
                    cancelOrder.side = common::OrderSide::Buy;
                } else if (sideChar == fix::ORDER_SIDE_SELL) {
                    cancelOrder.side = common::OrderSide::Sell;
                } else {
                    LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Invalid Side: {} in message: {}", sideChar, fixMessage);
                    return std::nullopt;
                }
            } else {
                LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Malformed Side tag in message: {}", fixMessage);
                return std::nullopt;
            }
        } else {
            LOG_WARN("BinaryToCancelOrderRequestConverter::convert - Missing Side tag in message: {}", fixMessage);
            // TODO: Decide if Side is mandatory for CancelOrder. If so, return nullopt.
        }

        // TODO: Implement proper TransactTime (60) parsing from FIX message.
        // For now, using current system time as a placeholder.
        cancelOrder.transactTime = std::chrono::system_clock::now();

        LOG_INFO("BinaryToCancelOrderRequestConverter::convert - Parsed CancelOrder (skeleton). ClOrdID: {}, OrderID: {}", cancelOrder.clOrdID, cancelOrder.orderID);
        return cancelOrder;
    } catch (const std::exception& e) {
        LOG_ERROR("BinaryToCancelOrderRequestConverter::convert - Exception during parsing for message '{}': {}", fixMessage, e.what());
        return std::nullopt;
    }
}

} // namespace fix
