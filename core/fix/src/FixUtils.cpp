#include "common/Types.h"
#include "common_fix/Protocol.h"
#include <map>
#include <stdexcept>

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
        if (c == fix::ORDER_SIDE_BUY) return common::OrderSide::Buy;
        if (c == fix::ORDER_SIDE_SELL) return common::OrderSide::Sell;
        throw std::invalid_argument("Invalid OrderSide char");
    }

    common::OrderType charToOrderType(const char c)
    {
        if (c == fix::ORDER_TYPE_MARKET) return common::OrderType::Market;
        if (c == fix::ORDER_TYPE_LIMIT) return common::OrderType::Limit;
        throw std::invalid_argument("Invalid OrderType char");
    }

} // namespace fix
