cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

include(cmake/address_sanitizer.cmake)
include(cmake/thread_sanitizer.cmake)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(trhead_pool LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE INC "include/*.hpp")
file(GLOB_RECURSE SRC "src/*.cpp")

add_executable(main ${INC} ${SRC})

option(ENABLE_THREAD_SANITIZER "Enable Thread Sanitizer (TSan)" OFF)
option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer (ASan)" OFF)

if(ENABLE_THREAD_SANITIZER AND ENABLE_ADDRESS_SANITIZER)
    message(FATAL_ERROR "Thread Sanitizer and Address Sanitizer cannot be enabled simultaneously.")
endif()

if (ENABLE_THREAD_SANITIZER)
    add_thread_sanitizer(main)
endif()

if (ENABLE_ADDRESS_SANITIZER)
    add_address_sanitizer(main)
endif()

