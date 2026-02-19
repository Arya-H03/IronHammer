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

# ---------------- Source Files ----------------
file(GLOB_RECURSE IRONHAMMER_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Add TracyClient only in Debug
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND IRONHAMMER_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tracy/public/TracyClient.cpp"
    )
endif()

# ---------------- Executable ----------------
add_executable(IronHammer ${IRONHAMMER_SOURCES})

# ---------------- Include Directories ----------------
target_include_directories(IronHammer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tracy/public/tracy
)

# ---------------- Link Libraries ----------------
target_link_libraries(IronHammer PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    ImGui-SFML::ImGui-SFML
)

# ---------------- Tracy Integration ----------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(IronHammer PRIVATE TRACY_ENABLE)
endif()
