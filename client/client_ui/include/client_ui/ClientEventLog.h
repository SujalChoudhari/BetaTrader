#pragma once

#include "fix_client/FixClientSession.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <deque>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace client_ui {

    struct ClientLogEntry {
        std::string timestamp;
        fix_client::FixClientEventType type;
        std::string message;
    };

    class ClientEventLog {
    public:
        explicit ClientEventLog(std::size_t capacity = 500)
            : mCapacity(std::max<std::size_t>(1, capacity))
        {}

        void append(fix_client::FixClientEvent event)
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mEntries.push_back(
                    {timestamp(), event.type, std::move(event.message)});
            while (mEntries.size() > mCapacity) mEntries.pop_front();
        }

        std::vector<ClientLogEntry> snapshot() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return {mEntries.begin(), mEntries.end()};
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mEntries.clear();
        }

        std::size_t size() const
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return mEntries.size();
        }

    private:
        static std::string timestamp()
        {
            const auto now = std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now());
            std::tm localTime{};
            localtime_r(&now, &localTime);

            std::ostringstream result;
            result << std::put_time(&localTime, "%H:%M:%S");
            return result.str();
        }

        const std::size_t mCapacity;
        mutable std::mutex mMutex;
        std::deque<ClientLogEntry> mEntries;
    };

    inline const char* clientEventTypeName(fix_client::FixClientEventType type)
    {
        switch (type) {
        case fix_client::FixClientEventType::Session:
            return "SESSION";
        case fix_client::FixClientEventType::Reject:
            return "REJECT";
        case fix_client::FixClientEventType::ExecutionReport:
            return "EXECUTION";
        case fix_client::FixClientEventType::MarketData:
            return "MARKET DATA";
        case fix_client::FixClientEventType::TransportError:
            return "TRANSPORT";
        }
        return "EVENT";
    }

} // namespace client_ui
