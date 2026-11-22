cmake_minimum_required(VERSION 3.31)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy.
include(FetchContent)

# a expremental project for concurrency of mutex in POSIX standard.
project(ConcurrentExperiment VERSION 0.0.1 LANGUAGES C CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(fmt CONFIG REQUIRED)
find_package(Threads REQUIRED)

# Include
set(CPPCONCURRENT_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)

include_directories(${CPPCONCURRENT_SRC_INCLUDE_DIR})
include_directories(BEFORE src)

add_executable(ex1 src/atomicEx/atomic_experiment.cpp)
target_link_libraries(ex1 PRIVATE fmt::fmt)
target_compile_definitions(ex1 PRIVATE SUBOBJ)


add_executable(main src/main.cpp src/shared_counters.cpp)
set_source_files_properties(src/shared_counters.cpp
    PROPERTIES COMPILE_DEFINITIONS SUBOBJ)
target_link_libraries(main PRIVATE Threads::Threads)

