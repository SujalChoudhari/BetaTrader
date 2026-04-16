#pragma once

#include <string>
#include <memory>

// Forward declarations
struct GLFWwindow;
struct ImGuiContext;

namespace client_ui {

/**
 * @class UIManager
 * @brief Manages the lifecycle of the GLFW window and ImGui context.
 */
class UIManager {
public:
    UIManager();
    ~UIManager();

    /**
     * @brief Initializes GLFW, creates the window, and sets up ImGui.
     * @param title Window title.
     * @param width Initial width.
     * @param height Initial height.
     * @return true if successful.
     */
    bool start(const std::string& title, int width, int height);

    /**
     * @brief Shuts down ImGui and GLFW, destroying the window.
     */
    void stop();

    /**
     * @brief Checks if the window should close (e.g., user clicked X).
     */
    bool shouldClose() const;

    /**
     * @brief Prepares for a new frame (polls events, starts ImGui frame).
     */
    void beginFrame();

    /**
     * @brief Finalizes the frame (renders ImGui, swaps buffers).
     */
    void endFrame();

    /**
     * @brief Renders the master dockspace that fills the entire window.
     */
    void renderDockspace();

    GLFWwindow* getWindow() const { return mWindow; }

private:
    GLFWwindow* mWindow = nullptr;
    ImGuiContext* mImGuiContext = nullptr;
};

} // namespace client_ui
