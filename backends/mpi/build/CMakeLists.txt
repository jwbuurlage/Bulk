cmake_minimum_required(VERSION 3.0.0)

project(bulk_mpi)

include_directories(
    "../../../include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wfatal-errors -Wextra -g -O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

find_package(MPI QUIET)

if(MPI_CXX_FOUND)
    set(
        EXAMPLE_DIR
        "../examples/"
        )

    set(
        EXAMPLE_SOURCES
        "hello_mpi.cpp"
        )

    set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})

    foreach(source_file ${EXAMPLE_SOURCES})
        string(REPLACE ".cpp" "" source_name ${source_file})
        add_executable(${source_name} ${EXAMPLE_DIR}${source_file})
        target_link_libraries(${source_name} ${MPI_LIBRARIES})
    endforeach(source_file)

else(MPI_CXX_FOUND)
    message("No MPI installation found.")
endif(MPI_CXX_FOUND)
