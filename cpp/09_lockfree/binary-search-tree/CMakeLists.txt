cmake_minimum_required(VERSION 3.31)
project(binary_search_tree)

set(CMAKE_CXX_STANDARD 23)

#add_executable(binary_search_tree main.cpp
#        bst-locked-impl.h
#        bst-locked-impl-test.cpp)

find_package(GTest CONFIG REQUIRED COMPONENTS GTest GMock)
include_directories(${GTEST_INCLUDE_DIRS})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

add_executable(bst-locked-impl-test bst-locked-impl-test.cpp)
target_link_libraries(bst-locked-impl-test GTest::gtest GTest::gtest_main GTest::gmock)