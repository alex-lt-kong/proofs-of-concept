cmake_minimum_required(VERSION 3.31)
project(market-data-consumer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(spdlog CONFIG REQUIRED)

file(GLOB SOURCES "*.cpp")

include_directories(twsapi_macunix.1030.01/IBJts/source/cppclient/client)
add_subdirectory(twsapi_macunix.1030.01/IBJts/source/cppclient/client)
link_directories(${CMAKE_SOURCE_DIR})

add_executable(TestCppClient ${SOURCES})
set_target_properties(TestCppClient
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_link_libraries(TestCppClient PRIVATE twsapi bid spdlog::spdlog)
target_link_libraries(TestCppClient PUBLIC Threads::Threads)
target_include_directories(TestCppClient PRIVATE "${PROJECT_SOURCE_DIR}/source/cppclient/" "${PROJECT_SOURCE_DIR}/source/cppclient/client/")

if (MSVC)
    target_compile_options(TestCppClient PRIVATE /W3)
else ()
    target_compile_options(TestCppClient PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)
endif ()

install(TARGETS TestCppClient DESTINATION bin)

FILE(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION "include/tws-api")