#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ccc {

    struct ConfigEntries {
        std::vector<std::string> includes;
        std::vector<std::string> excludes;
    };

    ConfigEntries resolveConfig(
        const std::filesystem::path& rootDir,
        const std::filesystem::path& configPath
    );

    // Creates default config file in the given directory.
    // Returns:
    //  0 -> success
    //  1 -> already exists
    //  2 -> write error
    int createDefaultConfig(const std::filesystem::path& rootDir);

} // namespace ccc
