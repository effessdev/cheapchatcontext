#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct ProjectScanResult
{
    std::vector<std::filesystem::path> files;
    std::string tree;
};

ProjectScanResult ScanProject(const std::filesystem::path& root);
