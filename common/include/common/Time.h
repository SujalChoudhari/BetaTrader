//
// Created by sujal on 21-10-2025.
//

#pragma once
#include <chrono>

namespace common {
    /**
     * @brief A type alias for std::chrono::system_clock::time_point.
     *
     * This is used throughout the system to represent timestamps for orders, trades,
     * and other time-sensitive events.
     */
    using Timestamp = std::chrono::system_clock::time_point;
}
