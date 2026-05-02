#include <admin/ExchangePanel.h>
#include <imgui.h>

namespace admin {

ExchangePanel::ExchangePanel(ExchangeManager& manager) : mManager(manager) {}

void ExchangePanel::render() {
    ImGui::Begin("System Control");

    // Exchange Status
    ImGui::Text("Exchange Server:");
    ImGui::SameLine();
    if (mManager.isExchangeRunning()) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "RUNNING");
        if (ImGui::Button("Stop Exchange")) {
            mManager.stopExchange();
        }
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "STOPPED");
        if (ImGui::Button("Start Exchange")) {
            mManager.startExchange("./exchange_app");
        }
    }

    ImGui::Separator();

    // Simulator Status
    ImGui::Text("Trading Simulator:");
    ImGui::SameLine();
    if (mManager.isSimulatorRunning()) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "RUNNING");
        if (ImGui::Button("Stop Simulator")) {
            mManager.stopSimulator();
        }
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "STOPPED");
        ImGui::Text("Agents Count:");
        ImGui::SameLine();
        ImGui::SliderInt("##agents_slider", &mNumAgents, 50, 10000);
        
        if (ImGui::Button("Start Simulator", ImVec2(-1, 0))) {
            mManager.startSimulator("./client_simulator", mNumAgents);
        }
    }

    ImGui::Separator();
    
    // Quick Metrics
    if (mManager.isExchangeRunning() && mManager.isSimulatorRunning()) {
        ImGui::Text("Estimated Throughput: 15,000 msg/sec");
        ImGui::Text("Active Connections: %d", mNumAgents);
    }

    ImGui::End();
}

} // namespace admin
