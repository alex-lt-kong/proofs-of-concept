cmake_minimum_required(VERSION 3.31)
 project(TWS-API-PoC)
find_package(Threads REQUIRED)
file(GLOB SOURCES "*.cpp")

include_directories(client)
#find_library(NAMES gtest PATHS path1 path2 ... pathN)
link_directories(client/build/lib)
link_directories(${CMAKE_SOURCE_DIR})

find_package(spdlog CONFIG REQUIRED)

add_executable(TestCppClient ${SOURCES})
set_target_properties(TestCppClient
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_link_libraries(TestCppClient PRIVATE twsapi bid spdlog::spdlog)
target_link_libraries(TestCppClient PUBLIC Threads::Threads)
target_include_directories(TestCppClient PRIVATE "${PROJECT_SOURCE_DIR}/source/cppclient/" "${PROJECT_SOURCE_DIR}/source/cppclient/client/")

if( MSVC )
    target_compile_options( TestCppClient PRIVATE /W3 )
else()
    target_compile_options( TestCppClient PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter )
endif()

install(TARGETS TestCppClient DESTINATION bin)

FILE(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION "include/tws-api")