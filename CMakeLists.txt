cmake_minimum_required(VERSION 3.31)
include(FetchContent)

# a expremental project for concurrency of mutex in POSIX standard.
project(ConcurrentExperiment VERSION 0.0.1 C LANGUAGES CXX)

find_package(fmt CONFIG REQUIRED)

add_executable(ex1 src/atomicEx/atomic_experiment.cpp)

target_link_libraries(ex1 PRIVATE fmt::fmt)
