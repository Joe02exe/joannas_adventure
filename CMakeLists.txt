cmake_minimum_required(VERSION 3.10)

project(
        game
        VERSION 0.0.1
        DESCRIPTION "Advanced CPP project"
        LANGUAGES CXX
)

find_package(SFML CONFIG REQUIRED COMPONENTS Graphics)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE SFML::Graphics)