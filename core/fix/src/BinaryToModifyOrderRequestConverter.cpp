#include "fix/BinaryToModifyOrderRequestConverter.h"
#include "common/Instrument.h"
#include "common/Types.h"
#include "common_fix/Protocol.h"
#include "common_fix/Tags.h"
#include "fix/FixUtils.h"
#include "fix/ModifyOrderRequest.h"
#include "logging/Logger.h"
#include <chrono>
#include <map>
#include <optional>
#include <stdexcept>
#include <string_view>

namespace fix {

    std::optional<ModifyOrderRequest>
    BinaryToModifyOrderRequestConverter::convert(const std::string& fixMessage)
    {
        try {
            const auto tagMap = splitToMap(fixMessage, fix::SOH);

            ModifyOrderRequest request = {
                    std::stoull(std::string(
                            tagMap.at(static_cast<int>(fix::Tag::ClOrdID)))),
                    std::stoull(std::string(tagMap.at(
                            static_cast<int>(fix::Tag::OrigClOrdID)))),
                    std::stoull(std::string(
                            tagMap.at(static_cast<int>(fix::Tag::OrderID)))),
                    common::from_string(
                            tagMap.at(static_cast<int>(fix::Tag::Symbol))),
                    charToOrderSide(tagMap.at(static_cast<int>(fix::Tag::Side))
                                            .front()),
                    std::stoull(std::string(
                            tagMap.at(static_cast<int>(fix::Tag::OrderQty)))),
                    charToOrderType(
                            tagMap.at(static_cast<int>(fix::Tag::OrdType))
                                    .front()),
                    std::stod(std::string(
                            tagMap.at(static_cast<int>(fix::Tag::Price)))),
                    // TODO: Implement proper TransactTime (60) parsing from FIX
                    // message.
                    std::chrono::system_clock::now()};

            LOG_INFO("BinaryToModifyOrderRequestConverter::convert - Parsed "
                     "ModifyOrderRequest. ClOrdID: {}, OrigClOrdID: {}",
                     request.clOrdID, request.origClOrdID);
            return request;
        }
        catch (const std::exception& e) {
            LOG_ERROR("BinaryToModifyOrderRequestConverter::convert - "
                      "Exception during parsing for message '{}': {}",
                      fixMessage, e.what());
            return std::nullopt;
        }
    }

} // namespace fix
