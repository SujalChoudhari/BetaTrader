//
// Created by sujal on 29-10-2025.
//

#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <stdexcept>

namespace common {

    enum class OrderSymbol : uint8_t {
        EURUSD,
        USDJPY,
        GBPUSD,
        USDCAD,
        USDINR,
        EURINR,
        GBPINR,
        AUDJSD,
        USDMXN,
        COUNT // helper for bounds
    };

    constexpr std::array<std::string_view, static_cast<size_t>(OrderSymbol::COUNT)> symbol_names = {
        "EURUSD",
        "USDJPY",
        "GBPUSD",
        "USDCAD",
        "USDINR",
        "EURINR",
        "GBPINR",
        "AUDJSD",
        "USDMXN"
    };

    inline std::string to_string(OrderSymbol symbol) {
        return std::string(symbol_names[static_cast<size_t>(symbol)]);
    }


    inline OrderSymbol from_string(std::string_view name) {
        for (size_t i = 0; i < symbol_names.size(); ++i)
            if (symbol_names[i] == name)
                return static_cast<OrderSymbol>(i);
        throw std::invalid_argument("Unknown symbol name");
    }

}
