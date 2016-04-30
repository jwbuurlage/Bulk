cmake_minimum_required(VERSION 3.0.0)

project(bulk_test)

include_directories(
    "../../include"
    "../../ext/catch/include"
)

set(TEST_NAME "bulk_test")

set(
    TEST_SOURCES
    "../initialization.cpp"
    "../catch.cpp"
)

set(
    LIB_NAMES
    "pthread"
)

add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries( ${TEST_NAME} ${LIB_NAMES} )
