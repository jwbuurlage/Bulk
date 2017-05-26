cmake_minimum_required(VERSION 3.0.0)

set(BACKEND_NAME thread)
project(bulk_${BACKEND_NAME})

add_custom_target(${BACKEND_NAME})
add_definitions(-DBACKEND_THREAD)

include_directories(
    "../../../include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -Wfatal-errors -Wextra -g -O3 -pthread")

# Bulk examples that work for any backend

foreach(source_file ${EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(${BACKEND_NAME}_${source_name} ${EXAMPLE_DIR}${source_file})
    target_link_libraries(${BACKEND_NAME}_${source_name} )
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
endforeach(source_file)

# Benchmark executable

add_executable(${BACKEND_NAME}_benchmark "../../../benchmark/benchmark.cpp")
target_link_libraries(${BACKEND_NAME}_benchmark)
add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_benchmark)

# Bulk tests that work for any backend

add_executable(${BACKEND_NAME}_unittests ${TEST_SOURCES})
target_link_libraries(${BACKEND_NAME}_unittests)
add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_unittests)

# Examples specific for cpp backend

set(
    CPP_EXAMPLE_DIR
    "../examples/"
    )

set(
    CPP_EXAMPLE_SOURCES
    "hello_thread.cpp"
    "densematrix_sharedmemory.cpp"
    )

foreach(source_file ${CPP_EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(${BACKEND_NAME}_${source_name} ${CPP_EXAMPLE_DIR}${source_file})
    target_link_libraries( ${BACKEND_NAME}_${source_name} )
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
endforeach(source_file)
