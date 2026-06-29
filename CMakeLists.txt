cmake_minimum_required(VERSION 3.16)
project(ccc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CCC_COMMON_SOURCES
    src/main.cpp
    src/GitIgnore.cpp
    src/DirectoryScanner.cpp
    src/TreeRenderer.cpp
    src/Config.cpp
    src/MarkdownBuilder.cpp
    src/FileUtils.cpp
)

if(WIN32)
    set(CCC_PLATFORM_SOURCES src/Clipboard_Windows.cpp)
else()
    set(CCC_PLATFORM_SOURCES src/Clipboard_Linux.cpp)
endif()

add_executable(ccc ${CCC_COMMON_SOURCES} ${CCC_PLATFORM_SOURCES} "src/PathResolver.h" "src/PathResolver.cpp")

target_include_directories(ccc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

if(MSVC)
    target_compile_options(ccc PRIVATE /W4)
else()
    target_compile_options(ccc PRIVATE -Wall -Wextra)
endif()

# Some older GCC/libstdc++ versions need explicit linking for <filesystem>.
if(NOT WIN32)
    include(CheckCXXSourceCompiles)
    target_link_libraries(ccc PRIVATE $<$<CXX_COMPILER_ID:GNU>:stdc++fs>)
endif()
