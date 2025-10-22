cmake_minimum_required(VERSION 3.10)

project(
        game
        VERSION 0.0.1
        DESCRIPTION "Advanced CPP project"
        LANGUAGES CXX
)

find_package(SFML CONFIG REQUIRED COMPONENTS Graphics)
find_package(spdlog CONFIG REQUIRED)

find_package(GTest CONFIG REQUIRED)

add_executable(main src/main.cpp)
add_executable(test_all test/test.cpp)

enable_testing()

target_link_libraries(main PRIVATE SFML::Graphics)

target_link_libraries(main PRIVATE spdlog::spdlog)

target_link_libraries(test_all PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(AllTestsInMain test_all)