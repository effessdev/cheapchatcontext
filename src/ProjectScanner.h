#pragma once // A preprocessor directive that ensures this header file is included only once during compilation to prevent duplicate definition errors.

#include <filesystem>
#include <string>
#include <vector>

std::vector<std::filesystem::path> ScanProject(const std::filesystem::path &root);
