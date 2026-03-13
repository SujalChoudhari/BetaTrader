#include "fix/BinaryToCancelOrderRequestConverter.h"
#include "common/Instrument.h"
#include "common/Types.h"
#include "common_fix/FixUtils.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "common_fix/FixUtils.h"
#include "common_fix/CancelOrderRequest.h"
#include "logging/Logger.h"
#include <chrono>
#include <map>
#include <stdexcept>
#include <string_view>

namespace fix
{

std::optional<CancelOrderRequest> BinaryToCancelOrderRequestConverter::convert(const std::string& fixMessage)
{
    try {
        const auto tagMap = splitToMap(fixMessage, fix::SOH);

        CancelOrderRequest request = {
            std::stoull(std::string(tagMap.at(static_cast<int>(fix::Tag::ClOrdID)))),
            std::stoull(std::string(tagMap.at(static_cast<int>(fix::Tag::OrderID)))),
            common::from_string(tagMap.at(static_cast<int>(fix::Tag::Symbol))),
            charToOrderSide(tagMap.at(static_cast<int>(fix::Tag::Side)).front()),
            fix::parseTimestamp(std::string(tagMap.at(static_cast<int>(fix::Tag::TransactTime))))
        };

        LOG_INFO("BinaryToCancelOrderRequestConverter::convert - Parsed CancelOrderRequest. ClOrdID: {}, OrderID: {}", request.clOrdID, request.orderID);
        return request;

    } catch (const std::exception& e) {
        LOG_ERROR("BinaryToCancelOrderRequestConverter::convert - Exception during parsing for message '{}': {}", fixMessage, e.what());
        return std::nullopt;
    }
}

} // namespace fix
