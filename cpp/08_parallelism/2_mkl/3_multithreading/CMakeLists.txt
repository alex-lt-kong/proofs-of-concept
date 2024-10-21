cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(4_vector-ln LANGUAGES CXX C)

find_package(MKL CONFIG REQUIRED PATHS $ENV{MKLROOT})

if(CMAKE_C_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_C_FLAGS "/O2")
  set(CMAKE_CXX_FLAGS "/O2")
else()
  set(CMAKE_C_FLAGS "-Wall -Wextra -O3 -Ofast")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3 -Ofast")
endif()

add_library(
    rand
    SHARED
    rand.c
)
add_library(rand_static STATIC rand.c)

add_executable(mkl-impl mkl-impl.cpp)
target_link_libraries(mkl-impl
  rand_static
  MKL::MKL
)

add_executable(my-impl my-impl.cpp)
target_link_libraries(my-impl rand_static)

