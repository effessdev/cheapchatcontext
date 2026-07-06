# --- Automated Dependency Management ---
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

# --- Automated Test File Inclusion ---
# CONFIGURE_DEPENDS ensures CMake re-scans the folder when files are added/removed
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

# --- Test Setup ---
include(GoogleTest)

# Use the variable ${TEST_SOURCES} to compile all found files
add_executable(ccc_lib_tests ${TEST_SOURCES})

target_link_libraries(ccc_lib_tests PRIVATE ccc_lib GTest::gtest_main)

gtest_discover_tests(ccc_lib_tests)
