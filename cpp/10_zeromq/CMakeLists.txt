cmake_minimum_required(VERSION 3.30)
project(10_zeromq)

set(CMAKE_CXX_STANDARD 23)

find_package(cppzmq CONFIG REQUIRED)

add_executable(10_zeromq main.cpp
)

add_executable(pusher
        pusher.cpp
)
target_link_libraries(pusher
        PRIVATE cppzmq cppzmq-static
)

add_executable(puller
        puller.cpp
        utils.h
)
target_link_libraries(puller
        PRIVATE cppzmq cppzmq-static
)