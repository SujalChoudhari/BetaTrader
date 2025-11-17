#include "fix/BinaryToOrderRequestConverter.h"
#include "common/Instrument.h"
#include "fix/Protocol.h"
#include "fix/Tags.h"
#include <map>
#include <numeric>
#include <stdexcept>
#include <string_view>

namespace fix {

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

    common::OrderSide charToOrderSide(const char c)
    {
        if (c == ORDER_SIDE_BUY) return common::OrderSide::Buy;
        if (c == ORDER_SIDE_SELL) return common::OrderSide::Sell;
        throw std::invalid_argument("Invalid OrderSide char");
    }

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

        return {std::string(tagMap.at(static_cast<int>(Tag::SenderCompID))),
                std::stoull(
                        std::string(tagMap.at(static_cast<int>(Tag::ClOrdID)))),
                common::from_string(tagMap.at(static_cast<int>(Tag::Symbol))),
                charToOrderSide(tagMap.at(static_cast<int>(Tag::Side)).front()),
                std::stod(std::string(
                        tagMap.at(static_cast<int>(Tag::OrderQty)))),
                std::stod(
                        std::string(tagMap.at(static_cast<int>(Tag::Price))))};
    }

} // namespace fix
