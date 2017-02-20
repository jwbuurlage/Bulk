cmake_minimum_required(VERSION 3.0.0)

project(bulk)

add_subdirectory("../backends/mpi/build" "../bin/mpi" EXCLUDE_FROM_ALL)
add_subdirectory("../backends/cpp/build" "../bin/cpp" EXCLUDE_FROM_ALL)
