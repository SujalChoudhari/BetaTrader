cmake_minimum_required(VERSION 3.15)
project(client_ui LANGUAGES CXX)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/lib/imgui)
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/vendor/lib/implot)

# ImGui Source Files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImPlot Source Files
set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

# client_ui Source Files
set(UI_SOURCES
    src/ChartPanel.cpp
    src/ConnectionPanel.cpp
    src/OrderBookPanel.cpp
    src/SimulatorPanel.cpp
    src/Theme.cpp
    src/TradingPanel.cpp
    src/UIManager.cpp
)

add_library(client_ui_lib STATIC
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    ${UI_SOURCES}
)

target_include_directories(client_ui_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
    ${CMAKE_SOURCE_DIR}/vendor/lib/asio/include
)

target_link_libraries(client_ui_lib
    PUBLIC
    common
    glfw
    client_fix_lib
    client_ohlc_lib
    client_orderbook_lib
    client_simulator_lib
)

# Setup OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(client_ui_lib PUBLIC OpenGL::GL)

# ImGui's imgui_impl_opengl3.cpp includes a built-in GL loader when no
# external loader (GLAD, GL3W, etc.) is defined. No extra definition needed.

target_compile_definitions(client_ui_lib PUBLIC IMGUI_ENABLE_DOCKING)
