cmake_minimum_required(VERSION 3.31)
project(binary_search_tree)

set(CMAKE_CXX_STANDARD 23)

add_executable(binary_search_tree main.cpp
        bst-locked-impl.h)
