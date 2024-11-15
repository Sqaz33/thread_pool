cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(trhead_pool LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE INC "include/*.hpp")
file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(main ${INC} ${SRC})
