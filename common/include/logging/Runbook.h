#pragma once

#include "spdlog/fmt/fmt.h"
#include "spdlog/spdlog.h" // needed for get_default()
#include <stdexcept>
#include <string>
#include "logging/Logger.h"
#include <string_view>

namespace runbook {
    class ErrorDefinition {
    public:
        consteval ErrorDefinition(const std::string_view code,
                                  const std::string_view what,
                                  const std::string_view how)
            : mCode(code), mWhatWentWrongString(what), mHowToFixString(how)
        {}

        [[nodiscard]] std::string_view getCode() const { return mCode; }
        [[nodiscard]] std::string_view getWhat() const
        {
            return mWhatWentWrongString;
        }
        [[nodiscard]] std::string_view getHow() const
        {
            return mHowToFixString;
        }

    private:
        std::string_view mCode;
        std::string_view mWhatWentWrongString;
        std::string_view mHowToFixString;
    };

    /**
     * @brief Internal helper function to format the final log message.
     */
    template<typename... Args>
    static std::string FormatRunbookLog(const ErrorDefinition& errorDef,
                                        const std::string_view& fmt_str,
                                        Args&&... args)
    {
        std::string user_msg = fmt::format(fmt::runtime(fmt_str),
                                           std::forward<Args>(args)...);

        std::string_view code = errorDef.getCode();
        std::string_view what = errorDef.getWhat();
        std::string_view how = errorDef.getHow();

        return fmt::format("[{}] {}\n"
                           "    [?] What Went Wrong: {}\n"
                           "    [!] How to Fix: {}",
                           code, user_msg, what, how);
    }
} // namespace runbook

// --- REDEFINE LOGGING MACROS ---

#ifdef LOG_ERROR
#undef LOG_ERROR
#endif

#ifdef LOG_CRITICAL
#undef LOG_CRITICAL
#endif

/** * @brief Logs an error message with a runbook code. */
#define LOG_ERROR(code, ...)                                                   \
    do {                                                                       \
        if (spdlog::default_logger()->should_log(spdlog::level::err)) {        \
            spdlog::source_loc loc{__FILE__, __LINE__, __FUNCTION__};          \
            std::string msg = runbook::FormatRunbookLog(code, __VA_ARGS__);    \
            spdlog::default_logger()->log(loc, spdlog::level::err,             \
                                          spdlog::string_view_t(msg));         \
        }                                                                      \
    }                                                                          \
    while (0)

/** * @brief Logs a critical message with a runbook code. */
#define LOG_CRITICAL(code, ...)                                                \
    do {                                                                       \
        if (spdlog::default_logger()->should_log(spdlog::level::critical)) {   \
            spdlog::source_loc loc{__FILE__, __LINE__, __FUNCTION__};          \
            std::string msg = runbook::FormatRunbookLog(code, __VA_ARGS__);    \
            spdlog::default_logger()->log(loc, spdlog::level::critical,        \
                                          spdlog::string_view_t(msg));         \
        }                                                                      \
    }                                                                          \
    while (0)
