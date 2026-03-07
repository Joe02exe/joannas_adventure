cmake_minimum_required(VERSION 3.10)

project(
        game
        VERSION 0.0.1
        DESCRIPTION "Advanced CPP project"
        LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(MIYOO_BUILD "Build for Miyoo Mini Plus" OFF)
option(SDL1_BUILD "Build native SDL1 version" OFF)

if(MIYOO_BUILD OR SDL1_BUILD)
    # The Miyoo cross-compiler sysroot already knows where SDL2 is,
    # so we don't need find_package or PkgConfig.
else()
    # SFML3 uses namespace targets: SFML::Graphics etc.
    set(SFML_STATIC_LIBRARIES ON)
    find_package(SFML 3 REQUIRED COMPONENTS Graphics Window Audio System)
endif()

if(MIYOO_BUILD OR SDL1_BUILD)
    include(FetchContent)
    message(STATUS "Miyoo Build: Fetching spdlog and nlohmann_json...")
    
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.12.0
    )
    FetchContent_GetProperties(spdlog)
    if(NOT spdlog_POPULATED)
        FetchContent_Populate(spdlog)
        add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
    endif()
    
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.2
    )
    FetchContent_GetProperties(nlohmann_json)
    if(NOT nlohmann_json_POPULATED)
        FetchContent_Populate(nlohmann_json)
        add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
else()
    # Use FetchContent for spdlog and json on PC too (not always available as system CMake packages)
    include(FetchContent)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.12.0
    )
    FetchContent_GetProperties(spdlog)
    if(NOT spdlog_POPULATED)
        FetchContent_Populate(spdlog)
        add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
    endif()
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.2
    )
    FetchContent_GetProperties(nlohmann_json)
    if(NOT nlohmann_json_POPULATED)
        FetchContent_Populate(nlohmann_json)
        add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

# add all sources from src/ (re-evaluated on CMake configure)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/test/*.cpp"
)

add_executable(main ${SRC_FILES})
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/src")

set(GAME_SOURCES ${SRC_FILES})
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

  # define macros for main
target_compile_definitions(main PRIVATE
    LOGGING_ENABLED=0
    IMGUI_ENABLED=0
    GOD_MODE_ENABLED=1
    INFINITE_RESOURCES_ENABLED=1
)

if(MIYOO_BUILD OR SDL1_BUILD)
    target_compile_definitions(main PRIVATE MIYOO_BUILD=1)
endif()

if(MIYOO_BUILD)
    
    # Point the compiler explicitly to the SDL headers inside the toolchain sysroot.
    target_include_directories(main PRIVATE /opt/miyoomini-toolchain/usr/arm-linux-gnueabihf/sysroot/usr/include/SDL)
    
    # Directly link the SDL libraries provided by the Miyoo sysroot
    target_link_libraries(main PRIVATE SDL SDL_image SDL_mixer SDL_ttf pthread stdc++fs)
    target_link_libraries(main PRIVATE spdlog::spdlog)
    target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
elseif(SDL1_BUILD)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL1 REQUIRED sdl SDL_image SDL_mixer SDL_ttf)
    target_include_directories(main PRIVATE ${SDL1_INCLUDE_DIRS})
    target_link_libraries(main PRIVATE ${SDL1_LIBRARIES} pthread stdc++fs)
    target_link_libraries(main PRIVATE spdlog::spdlog)
    target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
else()
    # SFML3 targets
    target_link_libraries(main PRIVATE SFML::Graphics SFML::Window SFML::Audio SFML::System)
    target_link_libraries(main PRIVATE spdlog::spdlog)
    target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
endif()

# Unit tests disabled for PC build (requires SFML3 + ImGui-SFML + GTest)
# Run via the Miyoo container or CI instead.

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(main copy_assets)
