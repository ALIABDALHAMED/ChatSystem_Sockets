cmake_minimum_required(VERSION 3.10)
project(ChatSystem_Sockets)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ====================================================================
# Find required packages
# ====================================================================
find_package(Threads REQUIRED)

# ====================================================================
# Common include directories
# ====================================================================
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ====================================================================
# Server executable
# ====================================================================
add_executable(server
    src/server.cpp
)

target_link_libraries(server
    PRIVATE
    Threads::Threads
    ws2_32
)

# ====================================================================
# Command-line Client executable
# ====================================================================
add_executable(client
    src/client.cpp
    src/networking/ChatClient.cpp
)

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(client
    PRIVATE
    Threads::Threads
    ws2_32
)

# ====================================================================
# GUI Client executable (ImGui + GLFW) - Optional
# ====================================================================

# Try to find GLFW3 and OpenGL
find_package(OpenGL QUIET)
find_package(glfw3 CONFIG QUIET)

# If not found via CMake, try to find it in a custom location
if(NOT glfw3_FOUND)
    # Check for manual GLFW installation
    if(EXISTS "C:/libs/glfw/include/GLFW/glfw3.h")
        message(STATUS "Found GLFW in C:/libs/glfw")
        set(GLFW_FOUND TRUE)
        set(GLFW_INCLUDE_DIR "C:/libs/glfw/include")
        set(GLFW_LIB_DIR "C:/libs/glfw/lib")
    else()
        set(GLFW_FOUND FALSE)
    endif()
else()
    set(GLFW_FOUND TRUE)
endif()

# Build ChatGUI if both GLFW and OpenGL are available
if(GLFW_FOUND AND OPENGL_FOUND)
    message(STATUS "Building ChatGUI target")

    add_executable(ChatGUI
        gui/main_gui.cpp
        src/gui/ChatGui.cpp
        src/networking/ChatClient.cpp
        gui/imgui/imgui.cpp
        gui/imgui/imgui_draw.cpp
        gui/imgui/imgui_tables.cpp
        gui/imgui/imgui_widgets.cpp
        gui/imgui/imgui_impl_glfw.cpp
        gui/imgui/imgui_impl_opengl3.cpp
    )

    target_include_directories(ChatGUI
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/gui/imgui
    )

    if(glfw3_FOUND)
        # Using CMake-found GLFW3
        target_link_libraries(ChatGUI
            PRIVATE
            glfw
            OpenGL::OpenGL
            Threads::Threads
            ws2_32
        )
    else()
        # Using manual GLFW installation
        target_include_directories(ChatGUI PRIVATE ${GLFW_INCLUDE_DIR})
        target_link_libraries(ChatGUI
            PRIVATE
            "${GLFW_LIB_DIR}/libglfw3.a"
            opengl32
            Threads::Threads
            ws2_32
        )
    endif()

else()
    message(STATUS "ChatGUI target will NOT be built")
    message(STATUS "  Reason:")
    if(NOT OPENGL_FOUND)
        message(STATUS "    - OpenGL not found (should be available on Windows)")
    endif()
    if(NOT GLFW_FOUND)
        message(STATUS "    - GLFW3 not found")
        message(STATUS "    - Option 1: Install via vcpkg:")
        message(STATUS "      vcpkg install glfw3:x64-windows")
        message(STATUS "    - Option 2: Download GLFW and build manually")
        message(STATUS "    - Option 3: Place GLFW in C:/libs/glfw/")
    endif()
endif()

# ====================================================================
# Compiler-specific settings
# ====================================================================
if(MSVC)
    # Visual Studio
    target_compile_options(server PRIVATE /W4)
    target_compile_options(client PRIVATE /W4)
    if(TARGET ChatGUI)
        target_compile_options(ChatGUI PRIVATE /W4)
    endif()
else()
    # GCC/Clang (MinGW)
    target_compile_options(server PRIVATE -Wall -Wextra)
    target_compile_options(client PRIVATE -Wall -Wextra)
    if(TARGET ChatGUI)
        target_compile_options(ChatGUI PRIVATE -Wall -Wextra)
    endif()
endif()
