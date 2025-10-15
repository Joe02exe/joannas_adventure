cmake_minimum_required(VERSION 3.10)

project(
        game
        VERSION 0.0.1
        DESCRIPTION "Advanced CPP project"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)

# --- Application ---
add_executable(main
        src/main.cpp
)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(main PRIVATE OpenGL::GL glfw GLEW::GLEW glm::glm)

