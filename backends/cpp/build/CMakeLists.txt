cmake_minimum_required(VERSION 3.0.0)

project(bulk_examples)

include_directories(
    "../../../include"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wfatal-errors -Wextra -g -O3 -pthread")

add_custom_target(cpp)
add_definitions(-DBACKEND_CPP)

# Bulk examples that work for any backend

foreach(source_file ${EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(cpp_${source_name} ${EXAMPLE_DIR}${source_file})
    target_link_libraries(cpp_${source_name} )
    add_dependencies(cpp cpp_${source_name})
endforeach(source_file)

# Benchmark executable

add_executable(cpp_benchmark "../../../benchmark/benchmark.cpp")
target_link_libraries(cpp_benchmark)
add_dependencies(cpp cpp_benchmark)

# Bulk tests that work for any backend

add_executable(cpp_test ${TEST_SOURCES})
target_link_libraries(cpp_test)
add_dependencies(cpp cpp_test)

# Examples specific for cpp backend

set(
    CPP_EXAMPLE_DIR
    "../examples/"
    )

set(
    CPP_EXAMPLE_SOURCES
    "hello_cpp.cpp"
    )

foreach(source_file ${CPP_EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(${source_name} ${CPP_EXAMPLE_DIR}${source_file})
    target_link_libraries( ${source_name} )
    add_dependencies(cpp ${source_name})
endforeach(source_file)
