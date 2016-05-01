cmake_minimum_required(VERSION 3.0.0)

project(bulk_test)

link_directories(
    "../../ext/multicore-bsp/lib"
)

include_directories(
    "../../include"
    "../../ext/catch/include"
    "../../ext/multicore-bsp/include"
)

set(TEST_NAME "bulk_test")

set(
    TEST_SOURCES
    "../catch.cpp"
    "../initialization.cpp"
    "../communication.cpp"
)

set(
    LIB_NAMES
    "pthread"
    "mcbsp-compat1.2.0"
)

add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries( ${TEST_NAME} ${LIB_NAMES} )
