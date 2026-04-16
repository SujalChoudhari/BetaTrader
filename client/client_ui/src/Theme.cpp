#include "client_ui/Theme.h"
#include <imgui.h>

namespace client_ui {
namespace theme {

void applyBloombergTheme() {
    auto& style = ImGui::GetStyle();
    auto* colors = style.Colors;

    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.IndentSpacing = 20.0f;

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

    // --- Bloomberg-inspired Dark Palette ---
    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.05f, 0.08f, 1.00f);      // #0A0D14
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.07f, 0.10f, 1.00f);       // #10121A
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.09f, 0.12f, 0.95f);       // #14171F
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.12f, 0.16f, 1.00f);     // #1A1F29

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.95f, 1.00f);          // #E6EDF2
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.45f, 0.50f, 1.00f);  // #667380

    // Borders & Separators
    colors[ImGuiCol_Border] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);        // #262E38
    colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);

    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.11f, 0.14f, 0.20f, 1.00f);        // #1C2433
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.20f, 0.28f, 1.00f); // #293347
    colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.40f, 0.00f, 0.80f);  // Bloomberg Orange

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.40f, 0.00f, 1.00f);  // Bloomberg Orange

    // Frames (Inputs, etc)
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.12f, 0.16f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(1.00f, 0.40f, 0.00f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.40f, 0.00f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.14f, 0.20f, 1.00f);

    // Title Bg
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.14f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.04f, 0.05f, 0.08f, 1.00f);

    // Misc
    colors[ImGuiCol_DockingPreview] = ImVec4(1.00f, 0.40f, 0.00f, 0.60f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.40f, 0.00f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.40f, 0.00f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.40f, 0.00f, 1.00f);
}

} // namespace theme
} // namespace client_ui
