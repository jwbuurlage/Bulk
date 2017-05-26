cmake_minimum_required(VERSION 3.0.0)

set(BACKEND_NAME mpi)
project(bulk_${BACKEND_NAME})
find_package(MPI QUIET)

if(MPI_CXX_FOUND)
    include_directories(
        "../../../include"
        )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wfatal-errors -Wextra -Werror -g -O3")

    add_custom_target(${BACKEND_NAME})
    add_definitions(-DBACKEND_MPI)

    set(CMAKE_CXX_COMPILE_FLAGS ${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})

    foreach(source_file ${EXAMPLE_SOURCES})
        string(REPLACE ".cpp" "" source_name ${source_file})
        add_executable(${BACKEND_NAME}_${source_name} ${EXAMPLE_DIR}${source_file})
        target_link_libraries(${BACKEND_NAME}_${source_name} ${MPI_LIBRARIES})
        add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
    endforeach(source_file)

    # Benchmark executable

    add_executable(${BACKEND_NAME}_benchmark "../../../benchmark/benchmark.cpp")
    target_link_libraries(${BACKEND_NAME}_benchmark ${MPI_LIBRARIES})
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_benchmark)

    # Bulk tests that work for any backend

    add_executable(${BACKEND_NAME}_unittests ${TEST_SOURCES})
    target_link_libraries(${BACKEND_NAME}_unittests ${MPI_LIBRARIES})
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_unittests)

    # Examples specific for mpi backend
    set(
        MPI_EXAMPLE_DIR
        "../examples/"
        )

    set(
        MPI_EXAMPLE_SOURCES
        "hello_mpi.cpp"
        )

    foreach(source_file ${MPI_EXAMPLE_SOURCES})
        string(REPLACE ".cpp" "" source_name ${source_file})
        add_executable(${BACKEND_NAME}_${source_name} ${MPI_EXAMPLE_DIR}${source_file})
        target_link_libraries(${BACKEND_NAME}_${source_name} ${MPI_LIBRARIES})
        add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
    endforeach(source_file)

else(MPI_CXX_FOUND)
    message("No MPI installation found.")
endif(MPI_CXX_FOUND)
