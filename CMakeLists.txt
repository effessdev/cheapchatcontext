# Define the minimum version of CMake required for the project
cmake_minimum_required(VERSION 3.14)

# Set project name and basic information
project(ccc VERSION 1.0 LANGUAGES CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure dependencies before processing targets
# Force GoogleTest to use the Dynamic Runtime (MDd/MD) to match project settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Process source and test subdirectories
add_subdirectory(src)

# Enable CTest and process the test subdirectory
enable_testing()
add_subdirectory(tests)
