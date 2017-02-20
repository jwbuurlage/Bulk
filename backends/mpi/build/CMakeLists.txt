cmake_minimum_required(VERSION 3.0.0)

project(bulk_mpi)

include_directories(
    "../../../include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wfatal-errors -Wextra -Werror -g -O3")

find_package(MPI QUIET)

if(MPI_CXX_FOUND)
    add_definitions(-DBACKEND_MPI)
    set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})

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

    add_custom_target(mpi)

    foreach(source_file ${EXAMPLE_SOURCES})
        string(REPLACE ".cpp" "" source_name ${source_file})
        add_executable(mpi_${source_name} ${EXAMPLE_DIR}${source_file})
        target_link_libraries(mpi_${source_name} ${MPI_LIBRARIES})
        add_dependencies(mpi mpi_${source_name})
    endforeach(source_file)

    set(TEST_DIR "../../../test")

    set(
        TEST_SOURCES
        "${TEST_DIR}/communication.cpp"
        "${TEST_DIR}/initialization.cpp"
        "${TEST_DIR}/partitioning.cpp"
        "${TEST_DIR}/test_main.cpp"
    )

    add_executable(mpi_test ${TEST_SOURCES})
    target_link_libraries(mpi_test ${MPI_LIBRARIES})
    add_dependencies(mpi mpi_test)

    # Examples specific for mpi backend
    set(
        EXAMPLE_DIR
        "../examples/"
        )

    set(
        EXAMPLE_SOURCES
        "hello_mpi.cpp"
        )

    foreach(source_file ${EXAMPLE_SOURCES})
        string(REPLACE ".cpp" "" source_name ${source_file})
        add_executable(${source_name} ${EXAMPLE_DIR}${source_file})
        target_link_libraries(${source_name} ${MPI_LIBRARIES})
        add_dependencies(mpi ${source_name})
    endforeach(source_file)

else(MPI_CXX_FOUND)
    message("No MPI installation found.")
endif(MPI_CXX_FOUND)
