cmake_minimum_required(VERSION 3.23)
project(IronHammer LANGUAGES CXX)

# ---------------- C++ Standard ----------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------- Build Type ----------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Keep debug symbols even in Release for stack traces
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g")

# ---------------- Tracy Profiler ----------------
option(ENABLE_TRACY "Enable Tracy profiler" OFF)

# ---------------- FetchContent ----------------
include(FetchContent)

# -------- SFML 3.0.2 --------
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

# -------- ImGui --------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9
)
FetchContent_MakeAvailable(imgui)

# -------- ImGui-SFML --------
FetchContent_Declare(
    imgui_sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
)

set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

FetchContent_MakeAvailable(imgui_sfml)

# -------- Nlohmann JSON --------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

# -------- Backward-cpp --------
add_subdirectory(external/backward-cpp)

# ---------------- Source Files ----------------
file(GLOB_RECURSE IRONHAMMER_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Append Tracy if enabled
if(ENABLE_TRACY)
  list(APPEND IRONHAMMER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tracy/public/TracyClient.cpp"
    )
endif()

# ---------------- Executable ----------------
add_executable(IronHammer ${IRONHAMMER_SOURCES})

# ---------------- Compile Definitions ----------------
if(ENABLE_TRACY)
  target_compile_definitions(IronHammer PRIVATE TRACY_ENABLE)
endif()

# ---------------- Include Directories ----------------
target_include_directories(IronHammer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tracy/public/tracy
)

# ---------------- Compile Options ----------------
target_compile_options(IronHammer PRIVATE
    -g
    -fno-omit-frame-pointer
    -mavx2
)

# ---------------- Link Libraries ----------------
target_link_libraries(IronHammer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    ImGui-SFML::ImGui-SFML
    nlohmann_json::nlohmann_json
    backward
)
