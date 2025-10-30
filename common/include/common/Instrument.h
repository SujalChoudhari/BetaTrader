//
// Created by sujal on 29-10-2025.
//

#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <stdexcept>

namespace common {

    /**
     * @enum Instrument
     * @brief Represents the financial instruments available for trading.
     */
    enum class Instrument : uint8_t {
        EURUSD,  ///< Euro / US Dollar
        USDJPY,  ///< US Dollar / Japanese Yen
        GBPUSD,  ///< British Pound / US Dollar
        USDCAD,  ///< US Dollar / Canadian Dollar
        USDINR,  ///< US Dollar / Indian Rupee
        EURINR,  ///< Euro / Indian Rupee
        GBPINR,  ///< British Pound / Indian Rupee
        AUDJSD,  ///< Australian Dollar / US Dollar
        USDMXN,  ///< US Dollar / Mexican Peso
        COUNT    ///< Helper to get the number of instruments.
    };

    /**
     * @brief An array of string representations for the Instrument enum.
     */
    constexpr std::array<std::string_view, static_cast<size_t>(Instrument::COUNT)> symbol_names = {
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

    /**
     * @brief Converts an Instrument enum to its string representation.
     * @param symbol The Instrument to convert.
     * @return The string representation of the Instrument.
     */
    inline std::string to_string(Instrument symbol) {
        return std::string(symbol_names[static_cast<size_t>(symbol)]);
    }


    /**
     * @brief Converts a string to its corresponding Instrument enum.
     * @param name The string representation of the Instrument.
     * @return The Instrument enum.
     * @throws std::invalid_argument if the string is not a valid instrument.
     */
    inline Instrument from_string(std::string_view name) {
        for (size_t i = 0; i < symbol_names.size(); ++i)
            if (symbol_names[i] == name)
                return static_cast<Instrument>(i);
        throw std::invalid_argument("Unknown symbol name");
    }

}
