#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <stdexcept>

namespace trading_core {
    /**
     * @enum CommandType
     * @brief Represents the different types of commands that can be processed by the trading engine.
     */
    enum class CommandType : uint8_t {
        NewOrder,    ///< A command to create a new order.
        CancelOrder, ///< A command to cancel an existing order.
        ModifyOrder, ///< A command to modify an existing order.
        COUNT        ///< Helper to get the number of command types.
    };

    /**
     * @brief An array of string representations for the CommandType enum.
     */
    constexpr std::array<std::string_view, static_cast<size_t>(CommandType::COUNT)> command_type_names = {
        "NewOrder",
        "CancelOrder",
        "ModifyOrder"
    };

    /**
     * @brief Converts a CommandType enum to its string representation.
     * @param type The CommandType to convert.
     * @return The string representation of the CommandType.
     */
    inline std::string to_string(CommandType type) {
        return std::string(command_type_names[static_cast<size_t>(type)]);
    }

    /**
     * @brief Converts a string to its corresponding CommandType enum.
     * @param name The string representation of the CommandType.
     * @return The CommandType enum.
     * @throws std::invalid_argument if the string is not a valid command type.
     */
    inline CommandType from_string(std::string_view name) {
        for (size_t i = 0; i < command_type_names.size(); ++i)
            if (command_type_names[i] == name)
                return static_cast<CommandType>(i);
        throw std::invalid_argument("Unknown command type name");
    }
}
