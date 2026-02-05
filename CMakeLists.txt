cmake_minimum_required(VERSION 3.5)
project(IronHammer LANGUAGES CXX)

# ---------------- C++ Standard ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Avoid compiler-specific extensions

# ---------------- FetchContent ----------------
include(FetchContent)

# -------- SFML 3.0.2 --------
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# -------- ImGui 1.91.9 --------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9
)
FetchContent_MakeAvailable(imgui)

# -------- ImGui-SFML 3.0 (SFML 3 compatible) --------
FetchContent_Declare(
    imgui_sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
)
# Point ImGui-SFML to the fetched ImGui
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
FetchContent_MakeAvailable(imgui_sfml)

# ---------------- Executable ----------------
add_executable(IronHammer
    src/core/main.cpp
    
    # src/core/main.cpp 
    # src/core/EntityManager.hpp
    # src/core/Archetype.hpp
    # src/core/ECSCommon.h
)

# ---------------- Link Libraries ----------------
target_link_libraries(IronHammer
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
        ImGui-SFML::ImGui-SFML
)

# ---------------- Platform-Specific Settings (Optional) ----------------
# Example: Windows-specific libraries
# if(WIN32)
#     target_link_libraries(IronHammer PRIVATE user32 gdi32)
# elseif(UNIX)
#     target_link_libraries(IronHammer PRIVATE pthread dl)
# endif()
