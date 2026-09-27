#include "client_ui/ConnectionPanel.h"
#include <imgui.h>

namespace client_ui {

ConnectionPanel::ConnectionPanel() {}

ConnectionPanel::~ConnectionPanel() {}

void ConnectionPanel::render(std::shared_ptr<fix_client::FixClientSession>& session,
                              asio::io_context& ioContext,
                              MarketDataState& marketData) {
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
            configureSession(session);
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

    const auto sessionState = session ? session->getState() : fix_client::FixClientState::Disconnected;
    if (sessionState == fix_client::FixClientState::Disconnected
        || sessionState == fix_client::FixClientState::Connecting) {
        marketData.markDisconnected();
    } else {
        marketData.markSessionReady();
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), "Market Data");

    const std::string currentSymbol = marketData.symbol();
    int selectedSymbolIndex = 0;
    try {
        selectedSymbolIndex = static_cast<int>(common::from_string(currentSymbol));
    } catch (const std::invalid_argument&) {
        selectedSymbolIndex = 0;
    }

    const char* symbolNames[common::symbol_names.size()];
    for (size_t i = 0; i < common::symbol_names.size(); ++i) {
        symbolNames[i] = common::symbol_names[i].data();
    }
    if (ImGui::Combo("Symbol", &selectedSymbolIndex, symbolNames,
                     static_cast<int>(common::symbol_names.size()))) {
        if (marketData.status() == MarketDataSubscriptionStatus::Subscribed
            && session && sessionState == fix_client::FixClientState::Active) {
            session->sendMarketDataRequest(currentSymbol, '2');
        }
        marketData.selectSymbol(symbolNames[selectedSymbolIndex]);
    }

    const auto marketDataStatus = marketData.status();
    if (marketDataStatus == MarketDataSubscriptionStatus::Subscribed) {
        if (ImGui::Button("Unsubscribe", ImVec2(-1, 0))) {
            if (session && sessionState == fix_client::FixClientState::Active) {
                session->sendMarketDataRequest(marketData.symbol(), '2');
            }
            marketData.markSessionReady();
        }
    } else {
        const bool canSubscribe = session && marketData.canSubscribe(sessionState);
        if (!canSubscribe) ImGui::BeginDisabled();
        if (ImGui::Button("Subscribe", ImVec2(-1, 0))) {
            session->sendMarketDataRequest(marketData.symbol(), '1');
            marketData.markSubscribed();
        }
        if (!canSubscribe) ImGui::EndDisabled();
    }

    const auto finalMarketDataStatus = marketData.status();
    const char* marketDataStatusText = finalMarketDataStatus == MarketDataSubscriptionStatus::Subscribed
                                            ? "SUBSCRIBED"
                                        : finalMarketDataStatus == MarketDataSubscriptionStatus::Ready
                                            ? "READY"
                                            : "DISCONNECTED";
    ImGui::Text("Market data: %s", marketDataStatusText);

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
    if (ImGui::Button("Clear Logs")) mEventLog.clear();
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &mAutoScroll);
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& log : mEventLog.snapshot()) {
        ImVec4 color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
        if (log.type == fix_client::FixClientEventType::ExecutionReport) {
            color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
        } else if (log.type == fix_client::FixClientEventType::Reject
                   || log.type == fix_client::FixClientEventType::TransportError) {
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        } else if (log.type == fix_client::FixClientEventType::MarketData) {
            color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
        }
        ImGui::TextDisabled("[%s]", log.timestamp.c_str());
        ImGui::SameLine();
        ImGui::TextColored(color, "%s", clientEventTypeName(log.type));
        ImGui::SameLine();
        ImGui::TextWrapped("%s", log.message.c_str());
    }

    if (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void ConnectionPanel::configureSession(const std::shared_ptr<fix_client::FixClientSession>& session) {
    session->setEventCallback([this](const fix_client::FixClientEvent& event) {
        mEventLog.append(event);
    });
    session->setStateChangeCallback([this](fix_client::FixClientState state) {
        recordState(state);
    });
}

void ConnectionPanel::recordState(fix_client::FixClientState state) {
    const char* stateName = "UNKNOWN";
    switch (state) {
        case fix_client::FixClientState::Disconnected: stateName = "DISCONNECTED"; break;
        case fix_client::FixClientState::Connecting: stateName = "CONNECTING"; break;
        case fix_client::FixClientState::Connected: stateName = "CONNECTED"; break;
        case fix_client::FixClientState::LogonSent: stateName = "LOGON SENT"; break;
        case fix_client::FixClientState::Active: stateName = "ACTIVE"; break;
        case fix_client::FixClientState::LoggingOut: stateName = "LOGGING OUT"; break;
    }
    mEventLog.append({fix_client::FixClientEventType::Session,
                      std::string("Connection state: ") + stateName});
}

} // namespace client_ui
