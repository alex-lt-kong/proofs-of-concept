cmake_minimum_required(VERSION 3.25)
project(11_ipc_queue)

set(CMAKE_CXX_STANDARD 23)

find_package(cxxopts REQUIRED)
find_package(spdlog REQUIRED)
find_package(Protobuf REQUIRED)
#include_directories(${Protobuf_INCLUDE_DIRS})

set(PROTO_FILES src/test_payload.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_executable(producer src/producer.cpp ${PROTO_SRCS} ${PROTO_HDRS})
# generate C++ bindings residing in the build directory
target_include_directories(producer PUBLIC ${CMAKE_BINARY_DIR})
target_link_libraries(producer PRIVATE
        protobuf::libprotobuf
        spdlog::spdlog)

add_executable(consumer src/consumer.cpp ${PROTO_SRCS} ${PROTO_HDRS})
# generate C++ bindings residing in the build directory
target_include_directories(consumer PUBLIC ${CMAKE_BINARY_DIR})
target_link_libraries(consumer PRIVATE
        protobuf::libprotobuf
        spdlog::spdlog)