#include "client_ui/ConnectionPanel.h"
#include <imgui.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace client_ui {

ConnectionPanel::ConnectionPanel() {}

ConnectionPanel::~ConnectionPanel() {}

void ConnectionPanel::render(std::shared_ptr<fix_client::FixClientSession>& session, asio::io_context& ioContext) {
    ImGui::Begin("FIX Connection Control");

    // --- Connection Settings ---
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "Session Configuration");
    ImGui::InputText("Host", mHost, sizeof(mHost));
    ImGui::InputInt("Port", &mPort);
    ImGui::Separator();

    ImGui::InputText("SenderCompID", mSenderCompId, sizeof(mSenderCompId));
    ImGui::InputText("TargetCompID", mTargetCompId, sizeof(mTargetCompId));
    ImGui::SliderInt("Heartbeat (s)", &mHeartbeatInterval, 5, 60);
    ImGui::Checkbox("Force Sequence Reset", &mForceReset);
    
    ImGui::Spacing();

    // --- Lifecycle Buttons ---
    bool isDisconnected = !session || session->getState() == fix_client::FixClientState::Disconnected;
    bool isConnected = session && session->getState() == fix_client::FixClientState::Connected;
    bool isActive = session && session->getState() == fix_client::FixClientState::Active;

    if (isDisconnected) {
        if (ImGui::Button("Connect", ImVec2(-1, 0))) {
            // Always create a fresh session; old sockets can't be reused
            session = std::make_shared<fix_client::FixClientSession>(ioContext, mSenderCompId, mTargetCompId);
            session->connect(mHost, static_cast<short>(mPort));
        }

    } else {
        ImGui::BeginDisabled();
        ImGui::Button("Connected", ImVec2(-1, 0));
        ImGui::EndDisabled();
    }

    ImGui::Columns(2, nullptr, false);
    
    if (!isConnected && !isActive) ImGui::BeginDisabled();
    if (ImGui::Button("Logon (35=A)", ImVec2(-1, 0))) {
        session->sendLogon(mHeartbeatInterval, mForceReset);
    }
    if (!isConnected && !isActive) ImGui::EndDisabled();

    ImGui::NextColumn();

    if (!isActive) ImGui::BeginDisabled();
    if (ImGui::Button("Logout (35=5)", ImVec2(-1, 0))) {
        session->sendLogout("User requested logout");
    }
    if (!isActive) ImGui::EndDisabled();

    ImGui::Columns(1);

    if (!isDisconnected) {
        if (ImGui::Button("Disconnect (Force)", ImVec2(-1, 0))) {
            session->disconnect();
        }
    }

    ImGui::Separator();

    // --- Status Indicator ---
    fix_client::FixClientState state = isDisconnected ? fix_client::FixClientState::Disconnected : session->getState();
    ImVec4 statusColor;
    std::string stateStr;

    switch (state) {
        case fix_client::FixClientState::Disconnected: statusColor = ImVec4(1, 0, 0, 1); stateStr = "DISCONNECTED"; break;
        case fix_client::FixClientState::Connecting:   statusColor = ImVec4(1, 1, 0, 1); stateStr = "CONNECTING"; break;
        case fix_client::FixClientState::Connected:    statusColor = ImVec4(0, 1, 1, 1); stateStr = "CONNECTED (TCP UP)"; break;
        case fix_client::FixClientState::LogonSent:    statusColor = ImVec4(1, 0.5f, 0, 1); stateStr = "LOGON SENT"; break;
        case fix_client::FixClientState::Active:       statusColor = ImVec4(0, 1, 0, 1); stateStr = "ACTIVE (LOGGED IN)"; break;
        case fix_client::FixClientState::LoggingOut:   statusColor = ImVec4(1, 0.5f, 0, 1); stateStr = "LOGGING OUT"; break;
    }

    ImGui::Text("Session State: ");
    ImGui::SameLine();
    ImGui::TextColored(statusColor, "%s", stateStr.c_str());

    ImGui::End();

    // --- Message Log Window ---
    ImGui::Begin("FIX Message Log");
    if (ImGui::Button("Clear Logs")) mLogs.clear();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &mAutoScroll);
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& log : mLogs) {
        ImVec4 color = (log.direction == "IN") ? ImVec4(0.4f, 0.8f, 1.0f, 1.0f) : ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
        ImGui::TextDisabled("[%s]", log.timestamp.c_str());
        ImGui::SameLine();
        ImGui::TextColored(color, "%s", log.direction.c_str());
        ImGui::SameLine();
        ImGui::TextWrapped("%s", log.message.c_str());
    }

    if (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void ConnectionPanel::addLog(const std::string& direction, const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
    
    mLogs.push_back({ss.str(), direction, msg});
    if (mLogs.size() > 500) mLogs.erase(mLogs.begin());
}

} // namespace client_ui
