//
// Created by sujal on 28-10-2025.
//

#pragma once

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "spdlog/async.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace logging {
    class Logger {
    private:
        /**
         * @brief Generates a timestamped filename
         * @param baseLogPath Base path (e.g., "logs/app.log")
         * @return Timestamped path (e.g., "logs/app_2025-10-30_14-30-45.log")
         */
        static std::string GenerateTimestampedFilename(const std::string &baseLogPath) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");

            // Split the path into directory, name, and extension
            size_t lastSlash = baseLogPath.find_last_of("/\\");
            size_t lastDot = baseLogPath.find_last_of('.');

            std::string directory = (lastSlash != std::string::npos)
                ? baseLogPath.substr(0, lastSlash + 1) : "";
            std::string filename = (lastSlash != std::string::npos)
                ? baseLogPath.substr(lastSlash + 1) : baseLogPath;
            std::string extension = (lastDot != std::string::npos && lastDot > lastSlash)
                ? baseLogPath.substr(lastDot) : "";
            std::string basename = (lastDot != std::string::npos && lastDot > lastSlash)
                ? filename.substr(0, lastDot - (lastSlash + 1)) : filename;

            return directory + basename + "_" + ss.str() + extension;
        }

    public:
        /**
         * @brief Initializes the spdlog and sets up the sources and sinks
         * NOTE: Use Shutdown to clean up and dump all queue.
         *
         * @param loggerName Unique name for the logger
         * @param logFilePath Place to save the log file (timestamp will be added automatically)
         * @param globalLevel Level of log, default is trace
         * @param queueSize queue size of the logging queue
         * @param numThreads number of threads used by logger
         * @param maxFileSize maximum file size of the log file
         * @param maxFiles total number of files used by logger
         */
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

            // Generate timestamped filename
            std::string timestampedPath = GenerateTimestampedFilename(logFilePath);

            const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                timestampedPath, maxFileSize, maxFiles);
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
            asyncLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [t %t] [%^%l%$] [%s:%#] %v");


            spdlog::register_logger(asyncLogger);
            spdlog::set_default_logger(asyncLogger);

            spdlog::set_level(globalLevel);

            spdlog::set_error_handler([](const std::string &msg) {
                fprintf(stderr, "SPDLOG INTERNAL ERROR: %s\n", msg.c_str());
            });
        }

        /**
         * Clean up the logging threads and dump up the remaining logs in queue
         */
        static void Shutdown() {
            spdlog::shutdown();
        }
    };
}


#define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)