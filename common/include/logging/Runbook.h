#pragma once

#include "spdlog/fmt/fmt.h"
#include <string>
#include <string_view>


namespace runbook {
    /**
     * @brief A compile-time-defined error object.
     * This class replaces the Runbook registry. An instance of this
     * class *is* the runbook entry.
     */
    class ErrorDefinition {
    public:
        /**
         * @brief Defines a new runbook error.
         * 'consteval' (C++20) ensures this *must* be constructed at compile time.
         * If you are on C++17, use 'constexpr'.
         */
        consteval ErrorDefinition(const std::string_view code, const std::string_view what, const std::string_view how)
            : mCode(code), mWhatWentWrongString(what), mHowToFixString(how) {
        }

        std::string_view getCode() const { return mCode; }
        std::string_view getWhat() const { return mWhatWentWrongString; }
        std::string_view getHow() const { return mHowToFixString; }

    private:
        std::string_view mCode;
        std::string_view mWhatWentWrongString;
        std::string_view mHowToFixString;
    };


    /**
     * @brief Internal helper function to format the final log message.
     *
     * CHANGED: This now takes an ErrorDefinition object directly,
     * completely removing the need for a runtime map lookup.
     */
    template<typename... Args>
    static std::string FormatRunbookLog(const ErrorDefinition &errorDef, const std::string &fmt_str,
                                        Args &&... args) {
        std::string user_msg = fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));

        std::string_view code = errorDef.getCode();
        std::string_view what = errorDef.getWhat();
        std::string_view how = errorDef.getHow();

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
}

// --- REDEFINE LOGGING MACROS ---
#ifdef LOG_WARN
#undef LOG_WARN
#endif

#ifdef LOG_ERROR
#undef LOG_ERROR
#endif

#ifdef LOG_CRITICAL
#undef LOG_CRITICAL
#endif

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
