//
// Created by sujal on 28-10-2025.
//

#pragma once
#include <string>

#include "spdlog/async.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// This namespace just holds the Init/Shutdown class.
namespace logging {
    class Logger {
    public:
        static void Init(
            const std::string &loggerName = "async_logger",
            const std::string &logFilePath = "logs/app.log",
            const spdlog::level::level_enum globalLevel = spdlog::level::trace,
            const size_t queueSize = 8192,
            const size_t numThreads = 1,
            size_t maxFileSize = 1024 * 1024 * 10, // 10 MB
            size_t maxFiles = 5) {
            spdlog::init_thread_pool(queueSize, numThreads);

            std::vector<spdlog::sink_ptr> sinks;

            const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            consoleSink->set_level(globalLevel);
            sinks.push_back(consoleSink);

            const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFilePath, maxFileSize, maxFiles);
            fileSink->set_level(globalLevel);
            sinks.push_back(fileSink);

            const auto asyncLogger = std::make_shared<spdlog::async_logger>(
                loggerName,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );

            asyncLogger->set_level(globalLevel);

            // This is the default pattern for TRACE/INFO/DEBUG logs
            asyncLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [t %t] [%^%l%$] [%s:%#] %v");


            spdlog::register_logger(asyncLogger);
            spdlog::set_default_logger(asyncLogger);

            spdlog::set_level(globalLevel);

            spdlog::set_error_handler([](const std::string &msg) {
                fprintf(stderr, "SPDLOG INTERNAL ERROR: %s\n", msg.c_str());
            });
        }

        static void Shutdown() {
            spdlog::shutdown();
        }
    };
} // namespace logging

// --- BASE LOGGING MACROS ---
// These are the defaults. Runbook.h will override some of them.
#define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

