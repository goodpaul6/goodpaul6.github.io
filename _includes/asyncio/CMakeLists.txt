project(asyncio)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    socket.cpp
    io_context.cpp
    helpers.cpp
    timer.cpp
    unix_utils.cpp)

add_library(io ${SOURCES})

set(TEST_SOURCES
    test_main.cpp)

set(EXAMPLE_SOURCES
    example.cpp)

add_executable(test ${TEST_SOURCES})

add_executable(example_server ${EXAMPLE_SOURCES})
add_executable(example_client ${EXAMPLE_SOURCES})

target_link_libraries(test PRIVATE io)

target_link_libraries(example_server PRIVATE io)
target_link_libraries(example_client PRIVATE io)

target_compile_definitions(example_server PRIVATE SERVER)
