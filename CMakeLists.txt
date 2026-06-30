cmake_minimum_required(VERSION 3.28.0)
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
enable_testing()

# library
add_library(memory_manager SHARED
        src/memory_manager.c

        src/blocks.h
        src/blocks.c

        src/heap.h
        src/heap.c

        src/avl_tree.h
        src/avl_tree.c
)
target_compile_definitions(memory_manager PRIVATE DEBUG)

target_include_directories(memory_manager
    PUBLIC
        include/
)

# tests
add_executable(memory_manager_tests
        tests/test_memory_manager.cpp
)

# target_compile_definitions(memory_manager_tests PRIVATE DEBUG) # For debug

target_link_libraries(memory_manager_tests
    PRIVATE
        memory_manager
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(memory_manager_tests)
