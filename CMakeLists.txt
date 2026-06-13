cmake_minimum_required(VERSION 4.3.0)
project(memory_manager)

include(FetchContent)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# gtest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

# library
add_library(memory_manager SHARED
    src/memory_manager.c
)

target_include_directories(memory_manager
    PUBLIC
        include/
)

# tests
add_executable(memory_manager_tests
        tests/test_memory_manager.cpp
)

target_link_libraries(memory_manager_tests
    PRIVATE
        memory_manager
        GTest::gtest_main
)
