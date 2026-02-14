cmake_minimum_required(VERSION 3.23)
project(IronHammer LANGUAGES CXX)

# ---------------- C++ Standard ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE-COMMANDS OFF)
set(CMAKE_BUILD_TYPE Debug)
# ---------------- FetchContent ----------------
include(FetchContent)

# -------- SFML 3.0.2 --------
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Disable building SFML examples")
set(SFML_BUILD_TESTS OFF CACHE BOOL "Disable building SFML tests")
set(SFML_BUILD_DOC OFF CACHE BOOL "Disable building SFML documentation")

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

# -------- ImGui-SFML 3.0 --------
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
file(GLOB_RECURSE IRONHAMMER_SOURCES
     CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(IronHammer ${IRONHAMMER_SOURCES})

# ---------------- Link Directories ----------------
target_include_directories(IronHammer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
