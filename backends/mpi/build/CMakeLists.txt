cmake_minimum_required(VERSION 3.0.0)

set(BACKEND_NAME mpi)
project(bulk_${BACKEND_NAME})

find_package(MPI QUIET)

if(MPI_CXX_FOUND)
    add_library(bulk_mpi INTERFACE)
    target_include_directories(bulk_mpi INTERFACE ".." ${MPI_INCLUDE_PATH})
    target_compile_options(bulk_mpi INTERFACE ${MPI_COMPILE_FLAGS})
    target_link_libraries(bulk_mpi INTERFACE ${MPI_LIBRARIES} bulk)
    target_compile_definitions(bulk_mpi INTERFACE BACKEND_MPI)

    add_custom_target(${BACKEND_NAME})

    foreach(source_file ${EXAMPLE_SOURCES})
        string(REPLACE ".cpp" "" source_name ${source_file})
        add_executable(${BACKEND_NAME}_${source_name} ${EXAMPLE_DIR}${source_file})
        target_link_libraries(${BACKEND_NAME}_${source_name} bulk_mpi)
        target_compile_options(${BACKEND_NAME}_${source_name} PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
        add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
    endforeach(source_file)

    # PSC example

    add_executable(${BACKEND_NAME}_psc "../../../examples/psc/psc_example.cpp")
    target_link_libraries(${BACKEND_NAME}_psc bulk_mpi)
    target_include_directories(${BACKEND_NAME}_psc PRIVATE "../../../examples/psc/include")
target_compile_options(${BACKEND_NAME}_psc PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_psc)

    # Benchmark executable

    add_executable(${BACKEND_NAME}_benchmark "../../../benchmark/benchmark.cpp")
    target_link_libraries(${BACKEND_NAME}_benchmark bulk_mpi)
    target_compile_options(${BACKEND_NAME}_benchmark PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_benchmark)

    # Bulk tests that work for any backend

    add_executable(${BACKEND_NAME}_unittests ${TEST_SOURCES})
    target_link_libraries(${BACKEND_NAME}_unittests bulk_mpi)
    target_compile_options(${BACKEND_NAME}_unittests PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
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
        target_link_libraries(${BACKEND_NAME}_${source_name} bulk_mpi)
        target_compile_options(${BACKEND_NAME}_${source_name} PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
        add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
    endforeach(source_file)

else(MPI_CXX_FOUND)
    message("No MPI installation found.")
endif(MPI_CXX_FOUND)
