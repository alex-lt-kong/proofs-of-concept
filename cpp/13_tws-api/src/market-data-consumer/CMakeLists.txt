cmake_minimum_required(VERSION 3.31)
project(market-data-consumer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)

file(GLOB SOURCES "*.cpp")

link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(market-data-consumer ${SOURCES})
target_link_libraries(market-data-consumer PRIVATE spdlog::spdlog Threads::Threads)
target_link_libraries(market-data-consumer PUBLIC twsapi bid)
