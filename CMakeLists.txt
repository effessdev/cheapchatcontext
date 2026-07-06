# Define the minimum version of CMake required for the project
cmake_minimum_required(VERSION 3.14)

# Set project name and basic information
project(ccc VERSION 1.0 LANGUAGES CXX)

# Require C++20 standard; CMAKE_CXX_STANDARD_REQUIRED ensures an error if the compiler is too old
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes the specified subdirectories to process their respective CMakeLists.txt files
add_subdirectory(src)
add_subdirectory(tests)
