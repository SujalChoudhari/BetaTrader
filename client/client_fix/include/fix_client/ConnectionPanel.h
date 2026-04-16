#pragma once

#include "fix_client/FixClientSession.h"
#include <asio.hpp>
#include <memory>
#include <vector>
#include <string>

namespace fix_client {

/**
 * @class ConnectionPanel
 * @brief Provides a Dear ImGui interface for managing the FIX connection.
 */
class ConnectionPanel {
public:
    ConnectionPanel();
    ~ConnectionPanel();

    /**
     * @brief Renders the connection panel UI.
     * @param session Shared pointer to the FIX session.
     * @param ioContext Reference to the ASIO io_context (for starting connections).
     */
    void render(std::shared_ptr<FixClientSession>& session, asio::io_context& ioContext);

private:
    char mHost[128] = "127.0.0.1";
    int mPort = 8088;
    char mSenderCompId[64] = "CLIENT1";
    char mTargetCompId[64] = "BETA_EXCHANGE";
    int mHeartbeatInterval = 30;
    bool mForceReset = false;

    struct LogEntry {
        std::string timestamp;
        std::string direction; // "IN" or "OUT"
        std::string message;
    };
    std::vector<LogEntry> mLogs;
    bool mAutoScroll = true;

    void addLog(const std::string& direction, const std::string& msg);
};

} // namespace fix_client
