cmake_minimum_required(VERSION 3.31)
project(08_int_to_cha LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)
include_directories(${GTEST_INCLUDE_DIRS})

add_library(library STATIC library.c)

add_executable(benchmark
        benchmark.cpp
        library.h
)

target_link_libraries(benchmark
        PRIVATE library benchmark::benchmark benchmark::benchmark_main)


add_executable(library-test library-test.cpp)
target_link_libraries(library-test PRIVATE library GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(library-test)