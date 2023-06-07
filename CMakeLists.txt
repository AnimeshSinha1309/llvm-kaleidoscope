cmake_minimum_required(VERSION 3.2)

project(kaleidoscope)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(src)
add_subdirectory(test)

add_subdirectory(deps/spdlog)
add_subdirectory(deps/gtest)
add_subdirectory(deps/boost)
