# --- Automated Dependency Management ---
include(FetchContent)

# Define the source and version to download
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

# Downloads and adds the library to your project automatically
FetchContent_MakeAvailable(googletest)

# --- Test Setup ---
include(GoogleTest)

add_executable(ccc_lib_tests ProjectScannerTests.cpp)

# Link your scanner library and GTest
# GTest::gtest_main is provided by the FetchContent step above
target_link_libraries(ccc_lib_tests PRIVATE ccc_lib GTest::gtest_main)

# Register tests for CTest
gtest_discover_tests(ccc_lib_tests)
