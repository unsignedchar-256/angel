cmake_minimum_required(VERSION 3.31)
project(angel)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(
        -Wall
        -Wextra
        -O0
        --verbose
)


add_executable(angel main.cpp)

target_link_libraries(
        angel
        PUBLIC
        asound
)
