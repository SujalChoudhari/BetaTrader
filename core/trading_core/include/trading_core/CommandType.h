#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <stdexcept>

namespace trading_core {
    enum class CommandType : uint8_t {
        NewOrder,
        CancelOrder,
        ModifyOrder,
        COUNT
    };

    constexpr std::array<std::string_view, static_cast<size_t>(CommandType::COUNT)> command_type_names = {
        "NewOrder",
        "CancelOrder",
        "ModifyOrder"
    };

    inline std::string to_string(CommandType type) {
        return std::string(command_type_names[static_cast<size_t>(type)]);
    }

    inline CommandType from_string(std::string_view name) {
        for (size_t i = 0; i < command_type_names.size(); ++i)
            if (command_type_names[i] == name)
                return static_cast<CommandType>(i);
        throw std::invalid_argument("Unknown command type name");
    }
}
