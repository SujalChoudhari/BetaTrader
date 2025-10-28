#pragma once

#include "logging/Logger.h" // Include the base logger
#include "spdlog/fmt/fmt.h" // For string formatting
#include "spdlog/spdlog.h"  // For spdlog::default_logger, spdlog::source_loc, etc.
#include <string>
#include <string_view>
#include <memory> // For std::forward

namespace runbook {
    /**
     * @brief A compile-time-defined error object.
     * This class replaces the Runbook registry. An instance of this
     * class *is* the runbook entry.
     */
    class ErrorDefinition {
    private:
        std::string_view code_;
        std::string_view whatWentWrong_;
        std::string_view howToFix_;

    public:
        /**
         * @brief Defines a new runbook error.
         * 'consteval' (C++20) ensures this *must* be constructed at compile time.
         * If you are on C++17, use 'constexpr'.
         */
        consteval ErrorDefinition(std::string_view code, std::string_view what, std::string_view how)
            : code_(code), whatWentWrong_(what), howToFix_(how) {}

        // Getters for the formatting function
        std::string_view getCode() const { return code_; }
        std::string_view getWhat() const { return whatWentWrong_; }
        std::string_view getHow() const { return howToFix_; }
    };


    /**
     * @brief Internal helper function to format the final log message.
     *
     * CHANGED: This now takes an ErrorDefinition object directly,
     * completely removing the need for a runtime map lookup.
     */
    template<typename... Args>
    static std::string FormatRunbookLog(const runbook::ErrorDefinition& errorDef, const std::string &fmt_str, Args &&... args) {
        // 1. Format the user's custom message
        std::string user_msg = fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));

        // 2. Get details directly from the error object
        std::string_view code = errorDef.getCode();
        std::string_view what = errorDef.getWhat();
        std::string_view how = errorDef.getHow();

        // 3. Format the final, structured log message
        return fmt::format(
            "[{}] {}\n"
            "    [?] What Went Wrong: {}\n"
            "    [!] How to Fix: {}",
            code,
            user_msg,
            what,
            how
        );
    }
} // namespace runbook

// --- REDEFINE LOGGING MACROS ---
// NO CHANGES are needed here! The 'code' parameter in the macro
// will now just be the ErrorDefinition object, which is
// passed perfectly to the new FormatRunbookLog.

// We leave these as-is from Logger.h
// #define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
// #define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
// #define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)

// We override these to use our runbook system
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_CRITICAL

/**
 * @brief Logs a warning message with a runbook code.
 */
#define LOG_WARN(code, ...) do { \
    if (spdlog::default_logger()->should_log(spdlog::level::warn)) { \
        spdlog::source_loc loc{__FILE__, __LINE__, SPDLOG_FUNCTION}; \
        std::string msg = runbook::FormatRunbookLog(code, __VA_ARGS__); \
        spdlog::default_logger()->log(loc, spdlog::level::warn, spdlog::string_view_t(msg)); \
    } \
} while (0)

/**
 * @brief Logs an error message with a runbook code.
 */
#define LOG_ERROR(code, ...) do { \
    if (spdlog::default_logger()->should_log(spdlog::level::err)) { \
        spdlog::source_loc loc{__FILE__, __LINE__, SPDLOG_FUNCTION}; \
        std::string msg = runbook::FormatRunbookLog(code, __VA_ARGS__); \
        spdlog::default_logger()->log(loc, spdlog::level::err, spdlog::string_view_t(msg)); \
    } \
} while (0)

/**
 * @brief Logs a critical message with a runbook code.
 */
#define LOG_CRITICAL(code, ...) do { \
    if (spdlog::default_logger()->should_log(spdlog::level::critical)) { \
        spdlog::source_loc loc{__FILE__, __LINE__, SPDLOG_FUNCTION}; \
        std::string msg = runbook::FormatRunbookLog(code, __VA_ARGS__); \
        spdlog::default_logger()->log(loc, spdlog::level::critical, spdlog::string_view_t(msg)); \
    } \
} while (0)

