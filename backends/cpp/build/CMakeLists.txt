cmake_minimum_required(VERSION 3.0.0)

project(bulk_examples)

include_directories(
    "../../../include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wfatal-errors -Wextra -g -O3 -pthread")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../../../bin)

# Bulk examples that work for any backend

set(
    EXAMPLE_DIR
    "../../../examples/"
    )

set(
    EXAMPLE_SOURCES
    "communication.cpp"
    "dot_product.cpp"
    "hello.cpp"
    "messaging.cpp"
    "partitioning.cpp"
    )

foreach(source_file ${EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(cpp_${source_name} ${EXAMPLE_DIR}${source_file})
    target_link_libraries(cpp_${source_name} )
endforeach(source_file)

# Bulk tests that work for any backend


set(TEST_DIR "../../../test")

set(
    TEST_SOURCES
    "${TEST_DIR}/communication.cpp"
    "${TEST_DIR}/initialization.cpp"
    "${TEST_DIR}/partitioning.cpp"
    "${TEST_DIR}/test_main.cpp"
    )

add_executable(cpp_test ${TEST_SOURCES})
target_link_libraries(cpp_test)

# Examples specific for cpp backend

set(
    EXAMPLE_DIR
    "../examples/"
    )

set(
    EXAMPLE_SOURCES
    "hello_cpp.cpp"
    )

foreach(source_file ${EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(${source_name} ${EXAMPLE_DIR}${source_file})
    target_link_libraries( ${source_name} )
endforeach(source_file)
