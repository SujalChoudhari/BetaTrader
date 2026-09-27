#pragma once

#include "fix_client/FixClientSession.h"
#include "client_ui/ClientEventLog.h"
#include "client_ui/MarketDataState.h"
#include <asio.hpp>
#include <memory>
#include <string>

namespace client_ui {

/**
 * @class ConnectionPanel
 * @brief Provides a Dear ImGui interface for managing the FIX connection.
 *
 * This panel lives in client_ui (not client_fix) to keep the protocol
 * library free of GUI dependencies.
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
    void render(std::shared_ptr<fix_client::FixClientSession>& session,
                asio::io_context& ioContext,
                MarketDataState& marketData);

private:
    char mHost[128] = "127.0.0.1";
    int mPort = 8088;
    char mSenderCompId[64] = "CLIENT1";
    char mTargetCompId[64] = "BETA_EXCHANGE";
    int mHeartbeatInterval = 30;
    bool mForceReset = false;

    ClientEventLog mEventLog;
    bool mAutoScroll = true;

    void configureSession(const std::shared_ptr<fix_client::FixClientSession>& session);
    void recordState(fix_client::FixClientState state);
};

} // namespace client_ui
