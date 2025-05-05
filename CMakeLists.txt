cmake_minimum_required(VERSION 3.10)
project(wudsaqadi VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directory
include_directories(include)

# Gather all source files
file(GLOB_RECURSE SOURCES src/*.cpp)

# Define the executable
add_executable(wudsaqadi ${SOURCES} src/main.cpp)

# Set output directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Enable testing
enable_testing()