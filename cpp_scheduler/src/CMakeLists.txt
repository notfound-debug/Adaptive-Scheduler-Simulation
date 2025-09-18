cmake_minimum_required(VERSION 3.10)
project(CloudScheduler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers (relative to src/)
include_directories(../include)

# Collect all .cpp files from *this* folder (src/)
file(GLOB SOURCES "*.cpp")

# Define the executable
add_executable(scheduler ${SOURCES})
