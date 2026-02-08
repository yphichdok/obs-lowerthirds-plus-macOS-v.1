cmake_minimum_required(VERSION 3.16...3.26)

project(LowerThirdsPlus VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS specific settings
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks;@executable_path/../../../../Frameworks")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# OBS Source Path (for headers)
set(OBS_SOURCE_PATH "$ENV{HOME}/Development/obs-studio")

# OBS include directories from source
set(OBS_INCLUDE_DIRS
    "${OBS_SOURCE_PATH}/libobs"
)

# OBS libraries from installed OBS.app
set(OBS_APP_PATH "/Applications/OBS.app")
set(OBS_LIBRARY "${OBS_APP_PATH}/Contents/Frameworks/libobs.framework/libobs")

# Verify files exist
if(NOT EXISTS "${OBS_SOURCE_PATH}/libobs/obs-module.h")
    message(FATAL_ERROR "OBS source not found at ${OBS_SOURCE_PATH}")
endif()

if(NOT EXISTS "${OBS_LIBRARY}")
    message(FATAL_ERROR "OBS.app not found at ${OBS_APP_PATH}")
endif()

# Find nlohmann_json
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Source files (NO Qt dock)
set(PLUGIN_SOURCES
    src/plugin-main-simple.cpp
    src/lowerthirds-source-simple.cpp
    src/json-loader.cpp
)

set(PLUGIN_HEADERS
    src/lowerthirds-source-simple.hpp
    src/json-loader.hpp
)

# Create plugin library
add_library(LowerThirdsPlus MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# Include directories
target_include_directories(LowerThirdsPlus PRIVATE
    ${OBS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(LowerThirdsPlus PRIVATE
    nlohmann_json::nlohmann_json
    ${OBS_LIBRARY}
)

# Set bundle structure for macOS
set_target_properties(LowerThirdsPlus PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "plugin"
    OUTPUT_NAME "LowerThirdsPlus"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

# Install plugin
install(TARGETS LowerThirdsPlus
    LIBRARY DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins"
    BUNDLE DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins"
)

# Compiler warnings
target_compile_options(LowerThirdsPlus PRIVATE -Wall -Wextra -Wpedantic)

# Print configuration info
message(STATUS "=== LowerThirdsPlus Configuration ===")
message(STATUS "OBS Source: ${OBS_SOURCE_PATH}")
message(STATUS "OBS Library: ${OBS_LIBRARY}")
message(STATUS "=====================================")
