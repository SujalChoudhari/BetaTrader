//
// Created by sujal on 28-10-2025.
//

#pragma once

#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <vector> // Include for std::vector

namespace logging {
    class Logger {
    private:
        /**
         * @brief Generates a timestamped filename
         * @param baseLogPath Base path (e.g., "logs/app.log")
         * @return Timestamped path (e.g., "logs/app_2025-10-30_14-30-45.log")
         */
        static std::string
        GenerateTimestampedFilename(const std::string& baseLogPath)
        {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");

            // Split the path into directory, name, and extension
            size_t lastSlash = baseLogPath.find_last_of("/\\");
            size_t lastDot = baseLogPath.find_last_of('.');

            std::string directory
                    = (lastSlash != std::string::npos)
                              ? baseLogPath.substr(0, lastSlash + 1)
                              : "";
            std::string filename = (lastSlash != std::string::npos)
                                           ? baseLogPath.substr(lastSlash + 1)
                                           : baseLogPath;
            std::string extension
                    = (lastDot != std::string::npos && lastDot > lastSlash)
                              ? baseLogPath.substr(lastDot)
                              : "";
            std::string basename
                    = (lastDot != std::string::npos && lastDot > lastSlash)
                              ? filename.substr(0, lastDot - (lastSlash + 1))
                              : filename;

            return directory + basename + "_" + ss.str() + extension;
        }

    public:
        template<typename... Args>
        static void Log(spdlog::level::level_enum level,
                        const spdlog::source_loc location,
                        spdlog::format_string_t<Args...> format, Args&&... args)
        {
            std::shared_lock<std::shared_mutex> lock(logMutex());
            if (auto logger = spdlog::default_logger()) {
                logger->log(location, level, format,
                            std::forward<Args>(args)...);
            }
        }

        static void LogMessage(spdlog::level::level_enum level,
                               const spdlog::source_loc location,
                               const spdlog::string_view_t message)
        {
            std::shared_lock<std::shared_mutex> lock(logMutex());
            if (auto logger = spdlog::default_logger()) {
                logger->log(location, level, message);
            }
        }

        /**
         * @brief Initializes the spdlog and sets up the sources and sinks
         * NOTE: Use Shutdown to clean up and dump all queue.
         *
         * @param loggerName Unique name for the logger
         * @param logFilePath Place to save the log file (timestamp will be
         added automatically)
         * @param enableConsole to use the console based logging or not
         * @param enableFile to use the file based logging or not
         * @param globalLevel Level of log, default is trace
         * @param queueSize queue size of the logging queue
         * @param numThreads number of threads used by logger
         * @param maxFileSize maximum file size of the log file
         * @param maxFiles total number of files used by logger
         * @param customSink an optional custom sink to add (e.g., for testing)
         */
        static void
        Init(const std::string& loggerName = "async_logger",
             const std::string& logFilePath = "logs/app.log",
             const bool enableConsole = true, const bool enableFile = true,
             const spdlog::level::level_enum globalLevel = spdlog::level::trace,
             const size_t queueSize = 8192, const size_t numThreads = 1,
             size_t maxFileSize = 1024 * 1024 * 300, size_t maxFiles = 5,
             spdlog::sink_ptr customSink = nullptr)
        {
            std::unique_lock<std::shared_mutex> lock(logMutex());
            if (spdlog::get(loggerName)) { return; }

            if (!spdlog::thread_pool()) {
                spdlog::init_thread_pool(queueSize, numThreads);
            }
            std::vector<spdlog::sink_ptr> sinks;

            if (enableConsole) {
                auto consoleSink = std::make_shared<
                        spdlog::sinks::stdout_color_sink_mt>();
                consoleSink->set_level(globalLevel);
                sinks.push_back(consoleSink);
            }

            if (enableFile) {
                std::string timestampedPath
                        = GenerateTimestampedFilename(logFilePath);
                auto fileSink = std::make_shared<
                        spdlog::sinks::rotating_file_sink_mt>(
                        timestampedPath, maxFileSize, maxFiles);
                fileSink->set_level(globalLevel);
                sinks.push_back(fileSink);
            }

            if (customSink) { sinks.push_back(customSink); }

            if (sinks.empty()) {
                throw std::runtime_error(
                        "Logger requires at least one sink (file or console).");
            }

            auto asyncLogger = std::make_shared<spdlog::async_logger>(
                    loggerName, sinks.begin(), sinks.end(),
                    spdlog::thread_pool(),
                    spdlog::async_overflow_policy::block);

            asyncLogger->set_level(globalLevel);
            asyncLogger->set_pattern(
                    "[%Y-%m-%d %H:%M:%S.%e] [t %t] [%^%l%$] [%s:%#] %v");

            spdlog::register_logger(asyncLogger);
            spdlog::set_default_logger(asyncLogger);
            spdlog::set_level(globalLevel);
            spdlog::set_error_handler([](const std::string& msg) {
                fprintf(stderr, "SPDLOG INTERNAL ERROR: %s\n", msg.c_str());
            });
        }

        /**
         * Clean up the logging threads and dump up the remaining logs in queue
         */
        static void Shutdown()
        {
            std::unique_lock<std::shared_mutex> lock(logMutex());
            spdlog::shutdown();
        }

    private:
        static std::shared_mutex& logMutex()
        {
            static std::shared_mutex mutex;
            return mutex;
        }
    };
} // namespace logging

#define LOG_TRACE(...)                                                         \
    logging::Logger::Log(spdlog::level::trace,                                 \
                         spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, \
                         __VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
    logging::Logger::Log(spdlog::level::debug,                                 \
                         spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, \
                         __VA_ARGS__)
#define LOG_INFO(...)                                                          \
    logging::Logger::Log(spdlog::level::info,                                  \
                         spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, \
                         __VA_ARGS__)
#define LOG_WARN(...)                                                          \
    logging::Logger::Log(spdlog::level::warn,                                  \
                         spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, \
                         __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
    logging::Logger::Log(spdlog::level::err,                                   \
                         spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, \
                         __VA_ARGS__)
#define LOG_CRITICAL(...)                                                      \
    logging::Logger::Log(spdlog::level::critical,                              \
                         spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, \
                         __VA_ARGS__)
