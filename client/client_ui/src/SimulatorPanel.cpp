#include "client_ui/SimulatorPanel.h"
#include <imgui.h>
#include <vector>
#include "common/Instrument.h"

namespace client_ui {

    SimulatorPanel::SimulatorPanel() {}

    SimulatorPanel::~SimulatorPanel() {}

    void SimulatorPanel::render(simulator::StochasticSimulator* simulator) {
        ImGui::Begin("Simulator Dashboard");

        if (!simulator) {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Local Exchange: OFFLINE");
            ImGui::TextWrapped("The simulator requires a local exchange instance to operate. Please start the exchange from the Management Console.");
            ImGui::End();
            return;
        }

        bool isRunning = simulator->isRunning();
        
        if (isRunning) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "STATUS: RUNNING");
            ImGui::Text("Active Bots: %zu", simulator->getBotCount());
            ImGui::Text("Target Symbol: %s", mTargetSymbolIdx == 0 ? "ALL" : common::symbol_names[mTargetSymbolIdx - 1].data());
            if (ImGui::Button("Stop Simulator", ImVec2(-1, 0))) {
                simulator->stop();
            }
        } else {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "STATUS: READY");
            ImGui::InputInt("Target Bots", &mTargetBots);
            ImGui::SliderFloat("Intensity", &mIntensity, 0.1f, 10.0f);
            
            // Build symbol list: ALL + instruments
            std::vector<const char*> symbolNames;
            symbolNames.push_back("ALL");
            for (size_t i = 0; i < common::symbol_names.size(); ++i) {
                symbolNames.push_back(common::symbol_names[i].data());
            }
            
            ImGui::Combo("Symbol", &mTargetSymbolIdx, symbolNames.data(), symbolNames.size());
            
            if (ImGui::Button("Start Simulator", ImVec2(-1, 0))) {
                simulator->setBotCount(static_cast<size_t>(mTargetBots));
                simulator->setIntensity(mIntensity);
                if (mTargetSymbolIdx == 0) {
                    simulator->setSymbol("ALL");
                } else {
                    simulator->setSymbol(std::string(common::symbol_names[mTargetSymbolIdx - 1]));
                }
                simulator->start();
            }
        }

        ImGui::Separator();
        ImGui::Text("Info: When started, bots will place random-walk orders");
        ImGui::Text("for the selected target instrument(s).");

        ImGui::End();
    }

} // namespace client_ui
