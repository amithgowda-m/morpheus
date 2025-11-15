cmake_minimum_required(VERSION 3.12)
project(Morpheus VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native")

# Find dependencies
find_package(Threads REQUIRED)

# Include directories
include_directories(src/engine src/utils)

# Build engine library
add_library(morpheus-engine STATIC
    src/engine/graph/csr_graph.cpp
    src/engine/graph/graph_generator.cpp
    src/engine/algorithms/bfs.cpp
    src/engine/algorithms/pagerank.cpp
    src/engine/algorithms/betweenness.cpp
    src/engine/monitoring/performance_monitor.cpp
    src/utils/config_parser.cpp
    src/utils/result_writer.cpp
)

target_link_libraries(morpheus-engine PUBLIC Threads::Threads)

# Build tests
add_executable(test_graph tests/test_graph.cpp)
target_link_libraries(test_graph morpheus-engine)

add_executable(test_bfs tests/test_bfs.cpp)
target_link_libraries(test_bfs morpheus-engine)

add_executable(test_pagerank tests/test_pagerank.cpp)
target_link_libraries(test_pagerank morpheus-engine)

add_executable(test_integration tests/test_integration.cpp)
target_link_libraries(test_integration morpheus-engine)

add_executable(test_classifier_validation tests/test_classifier_validation.cpp)
target_link_libraries(test_classifier_validation morpheus-engine)


# Build benchmark runner
add_executable(benchmark-runner benchmarks/benchmark_runner.cpp)
target_link_libraries(benchmark-runner morpheus-engine)

enable_language(C)