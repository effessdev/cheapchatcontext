# --- Automated Dependency Management ---
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

# --- Test Setup ---
include(GoogleTest)

add_executable(ccc_lib_tests ProjectScannerTests.cpp FileFilterTests.cpp)

target_link_libraries(ccc_lib_tests PRIVATE ccc_lib GTest::gtest_main)

gtest_discover_tests(ccc_lib_tests)
