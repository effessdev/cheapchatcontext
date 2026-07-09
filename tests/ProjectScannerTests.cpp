#include <gtest/gtest.h>
#include "../src/ProjectScanner.h"
#include <filesystem>
#include <stdexcept>

TEST(ProjectScannerTest, ScanNonExistentDirectory)
{
    // Use a path that is guaranteed not to exist
    std::filesystem::path non_existent = "non_existent_dir_12345";

    // The function should throw a std::runtime_error
    EXPECT_THROW(ScanProject(non_existent), std::runtime_error);
}
