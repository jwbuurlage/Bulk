cmake_minimum_required(VERSION 3.0.0)
project(bulk_mpi_test)

include_directories(
    "../../../include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wfatal-errors -Wextra -g -O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

set(TEST_NAME "bulk_mpi_test")

find_package(MPI QUIET)

if(MPI_CXX_FOUND)
    set(
        TEST_SOURCES
        "../test_main.cpp"
        "../communication.cpp"
        "../initialization.cpp"
        )

    set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})

    include_directories(
        "../../../../include"
    )

    add_executable(${TEST_NAME} ${TEST_SOURCES})
    target_link_libraries(${TEST_NAME} ${MPI_LIBRARIES})
else(MPI_CXX_FOUND)
    message("No MPI installation found.")
endif(MPI_CXX_FOUND)
