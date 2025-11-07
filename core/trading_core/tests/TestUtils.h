/**
 * @file TestUtils.h
 * @brief Test utilities used by unit tests (in-memory log sink, helpers).
 *
 * Contains small helper types that make it easier to assert logging behavior
 * and other test-time conditions.
 */

#pragma once
#include <spdlog/sinks/base_sink.h>
#include <vector>
#include <string>
#include <mutex>

// A spdlog sink that stores log messages in memory for inspection in tests.
class MemorySink : public spdlog::sinks::base_sink<std::mutex> {
public:
    // Vector to store formatted log messages.
    std::vector<std::string> messages;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);
        messages.push_back(fmt::to_string(formatted));
    }

    void flush_() override {}
};