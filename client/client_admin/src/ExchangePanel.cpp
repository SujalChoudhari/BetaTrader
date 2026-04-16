#include "admin/ExchangePanel.h"
#include <imgui.h>
#include <iomanip>
#include <sstream>
#include <chrono>

namespace admin {

ExchangePanel::ExchangePanel(ExchangeManager& manager) : mManager(manager) {}

ExchangePanel::~ExchangePanel() {}

void ExchangePanel::render() {
    ImGui::Begin("Exchange Management Console");

    renderControlSection();
    ImGui::Separator();
    
    if (mManager.isRunning()) {
        if (ImGui::BeginTabBar("ExchangeTabs")) {
            if (ImGui::BeginTabItem("Sessions")) {
                renderSessionSection();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Engine Stats")) {
                renderEngineSection();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    } else {
        ImGui::TextDisabled("Start the exchange to view live statistics.");
    }

    ImGui::End();
}

void ExchangePanel::renderControlSection() {
    bool isRunning = mManager.isRunning();
    
    ImGui::Columns(2, "ControlHeader", false);
    
    if (isRunning) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "STATUS: ONLINE");
        if (ImGui::Button("Stop Exchange", ImVec2(-1, 0))) {
            mManager.stop();
        }
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "STATUS: OFFLINE");
        if (ImGui::Button("Start Exchange", ImVec2(-1, 0))) {
            mManager.start(static_cast<short>(mPort));
        }
    }

    ImGui::NextColumn();
    
    ImGui::PushItemWidth(-1);
    ImGui::Text("Server Port:");
    ImGui::InputInt("##port", &mPort);
    ImGui::PopItemWidth();

    ImGui::Columns(1);
}

void ExchangePanel::renderSessionSection() {
    auto* server = mManager.getServer();
    if (!server) return;

    const auto& sessionStates = server->getManager().getAllSessionStates();

    if (ImGui::BeginTable("SessionTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("SenderCompID");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("Next In Seq");
        ImGui::TableSetupColumn("Next Out Seq");
        ImGui::TableHeadersRow();

        for (const auto& [compId, state] : sessionStates) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", compId.c_str());
            
            ImGui::TableSetColumnIndex(1);
            if (state.isLoggedOn) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "LOGGED ON");
            } else {
                ImGui::TextDisabled("OFFLINE");
            }

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%u", state.inSeqNum);
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%u", state.outSeqNum);
        }
        ImGui::EndTable();
    }

}

void ExchangePanel::renderEngineSection() {
    auto* core = mManager.getCore();
    if (!core) return;

    ImGui::Text("Core Engine Partition Activity");
    ImGui::Separator();

    // In a real scenario, we'd iterate through partitions. 
    // Since partitions are internal to TradingCore, we'll show queue depths if exposed.
    // For now, let's just show a placeholder with the count of partitions.
    ImGui::Text("Partitions Active: %d", static_cast<int>(common::Instrument::COUNT));
    
    if (ImGui::BeginTable("PartitionTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Instrument (ID)");
        ImGui::TableSetupColumn("Queue Depth");
        ImGui::TableHeadersRow();

        for (int i = 0; i < static_cast<int>(common::Instrument::COUNT); ++i) {
             ImGui::TableNextRow();
             ImGui::TableSetColumnIndex(0);
             ImGui::Text("Instrument %d", i);
             ImGui::TableSetColumnIndex(1);
             ImGui::Text("0"); // Placeholder for queue depth
        }
        ImGui::EndTable();
    }
}

} // namespace admin
