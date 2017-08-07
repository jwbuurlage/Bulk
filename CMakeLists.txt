cmake_minimum_required(VERSION 3.0.0)

project(bulk)

# The example and test directories are relative to backends/xxx/build

set(EXAMPLE_DIR "../../../examples/")

set(
    EXAMPLE_SOURCES
    "word_count.cpp"
    "communication.cpp"
    "dot_product.cpp"
    "hello.cpp"
    "messaging.cpp"
    "partitioning.cpp"
    )

set(TEST_DIR "../../../test")

set(
    TEST_SOURCES
    "${TEST_DIR}/communication.cpp"
    "${TEST_DIR}/initialization.cpp"
    "${TEST_DIR}/partitioning.cpp"
    "${TEST_DIR}/test_main.cpp"
    )


add_subdirectory("backends/mpi/build" "bin/mpi" EXCLUDE_FROM_ALL)
add_subdirectory("backends/thread/build" "bin/thread" EXCLUDE_FROM_ALL)
