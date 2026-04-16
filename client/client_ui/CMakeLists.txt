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
file(GLOB_RECURSE UI_SOURCES "src/*.cpp")

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
)

# Setup OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(client_ui_lib PUBLIC OpenGL::GL)

# We need to define the GL loader (ImGui default is fine for GL3.3 if we don't use another one)
target_compile_definitions(client_ui_lib PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
# Wait, if I don't have GLAD, I should use something else. 
# Actually, GLFW + OpenGL usually works with a loader. 
# I'll use the default ImGui one which might need a header.
# For now let's assume the user has a loader or we'll add one.
# ImGui's opengl3 backend can use its own loader if IMGUI_IMPL_OPENGL_LOADER_CUSTOM is not set.

target_compile_definitions(client_ui_lib PUBLIC IMGUI_ENABLE_DOCKING)
