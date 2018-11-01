cmake_minimum_required(VERSION 3.8)

set(BACKEND_NAME thread)
project(bulk_${BACKEND_NAME})

add_library(bulk_thread INTERFACE)
target_include_directories(bulk_thread INTERFACE "..")
target_link_libraries(bulk_thread INTERFACE pthread bulk)
target_compile_definitions(bulk_thread INTERFACE BACKEND_THREAD)

add_custom_target(${BACKEND_NAME})

include_directories(
    "../../../include"
)

# Bulk examples that work for any backend

foreach(source_file ${EXAMPLE_SOURCES})
    string(REPLACE ".cpp" "" source_name ${source_file})
    add_executable(${BACKEND_NAME}_${source_name} ${EXAMPLE_DIR}${source_file})
    target_link_libraries(${BACKEND_NAME}_${source_name} bulk_thread)
    target_compile_options(${BACKEND_NAME}_${source_name} PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
endforeach(source_file)

# PSC example

add_executable(${BACKEND_NAME}_psc "../../../examples/psc/psc_example.cpp")
target_link_libraries(${BACKEND_NAME}_psc bulk_thread)
target_include_directories(${BACKEND_NAME}_psc PRIVATE "../../../examples/psc/include")
target_compile_options(${BACKEND_NAME}_psc PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_psc)

# Benchmark executable

add_executable(${BACKEND_NAME}_benchmark "../../../benchmark/benchmark.cpp")
target_link_libraries(${BACKEND_NAME}_benchmark bulk_thread)
target_compile_options(${BACKEND_NAME}_benchmark PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_benchmark)

# Bulk tests that work for any backend

add_executable(${BACKEND_NAME}_unittests ${TEST_SOURCES})
target_link_libraries(${BACKEND_NAME}_unittests bulk_thread)
target_compile_options(${BACKEND_NAME}_unittests PRIVATE ${BULK_SUGGESTED_COMPILE_OPTIONS})
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
    target_link_libraries( ${BACKEND_NAME}_${source_name} bulk_thread)
    add_dependencies(${BACKEND_NAME} ${BACKEND_NAME}_${source_name})
endforeach(source_file)

