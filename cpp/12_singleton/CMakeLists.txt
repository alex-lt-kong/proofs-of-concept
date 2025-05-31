cmake_minimum_required(VERSION 3.31)
project(12_my-singleton)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(singleton-benchmark singleton-benchmark.cpp
)

target_link_libraries(singleton-benchmark
        PRIVATE benchmark::benchmark benchmark::benchmark_main)


add_executable(singleton-test singleton-test.cpp)
target_link_libraries(singleton-test PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(singleton-test)