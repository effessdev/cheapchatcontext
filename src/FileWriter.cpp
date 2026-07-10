#include <fstream>
#include <filesystem>
#include <string>

#include "FileWriter.h"

namespace fs = std::filesystem;

void createConfig(const fs::path &folderPath)
{
    const std::string content = "your_specific_string_here";

    if (!fs::exists(folderPath))
    {
        fs::create_directories(folderPath);
    }

    std::ofstream configFile(folderPath / "ccc-config.toml", std::ios::out | std::ios::trunc);

    if (configFile.is_open())
    {
        configFile << content;
    }
}