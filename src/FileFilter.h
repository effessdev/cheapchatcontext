#pragma once

#include <filesystem>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

std::vector<fs::path> filterFiles(
    const std::vector<fs::path> &files,
    const std::vector<std::string> &includePatterns,
    const std::vector<std::string> &excludePatterns);
