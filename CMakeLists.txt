cmake_minimum_required(VERSION 3.10)

project(
        game
        VERSION 0.0.1
        DESCRIPTION "Advanced CPP project"
        LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Audio)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)

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

add_executable(unit_tests test/test_inventory.cpp ${TEST_FILES} ${GAME_SOURCES})

# define macros
target_compile_definitions(main PRIVATE
    LOGGING_ENABLED=1
    # for windows systems we have to disable the IMGUI window, statical / dynamic linking issues
    IMGUI_ENABLED=1
    GOD_MODE_ENABLED=1
    INFINITE_RESOURCES_ENABLED=1
)

target_compile_definitions(unit_tests PRIVATE
        LOGGING_ENABLED=0
        # for windows systems we have to disable the IMGUI window, statical / dynamic linking issues
        IMGUI_ENABLED=0
        GOD_MODE_ENABLED=0
        INFINITE_RESOURCES_ENABLED=0
)

enable_testing()

target_link_libraries(main PRIVATE SFML::Graphics SFML::Audio)

target_link_libraries(main PRIVATE spdlog::spdlog)

target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(main PRIVATE ImGui-SFML::ImGui-SFML)

target_link_libraries(unit_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
        SFML::Graphics
        SFML::Audio
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        ImGui-SFML::ImGui-SFML)

target_include_directories(unit_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(unit_tests PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
)

add_test(AllTestsInMain unit_tests)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

add_dependencies(main copy_assets)
add_dependencies(unit_tests copy_assets)