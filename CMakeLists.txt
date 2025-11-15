cmake_minimum_required(VERSION 3.16)
project(Morpheus VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3 -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O3")

# Find dependencies
find_package(Threads REQUIRED)

# Include directories
include_directories(src/engine)
include_directories(src/utils)

# Add subdirectories
add_subdirectory(src/engine)
add_subdirectory(tests)
add_subdirectory(benchmarks)

# Check for Intel PIN
if(EXISTS "$ENV{PIN_ROOT}")
    add_subdirectory(src/instrumentation)
endif()

# Installation
install(TARGETS morpheus-engine DESTINATION bin)
install(TARGETS benchmark-runner DESTINATION bin)