#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <nlohmann/json.hpp>

#include "Config.h"
#include "FileUtils.h"

namespace ccc {

ConfigEntries resolveConfig(const std::filesystem::path& rootDir,
    const std::filesystem::path& configPath) {
    (void)rootDir; // no longer used

    std::vector<std::string> includes;
    std::vector<std::string> excludes;

    std::error_code existsEc;
    if (!std::filesystem::exists(configPath, existsEc) || existsEc) {
        return {};
    }

    std::ifstream in(configPath, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Warning: could not open " << configPath.string() << "\n";
        return {};
    }

    nlohmann::json doc;
    try {
        in >> doc;
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Warning: failed to parse " << configPath.string()
            << ": " << e.what() << "\n";
        return {};
    }

    if (doc.contains("include") && doc["include"].is_array()) {
        for (const auto& v : doc["include"]) {
            if (v.is_string()) {
                includes.push_back(v.get<std::string>());
            }
        }
    }

    if (doc.contains("exclude") && doc["exclude"].is_array()) {
        for (const auto& v : doc["exclude"]) {
            if (v.is_string()) {
                excludes.push_back(v.get<std::string>());
            }
        }
    }

    return ConfigEntries{
        .includes = std::move(includes),
        .excludes = std::move(excludes)
    };
}

int createDefaultConfig(const std::filesystem::path& rootDir) {
    const std::filesystem::path configPath = rootDir / "ccc.config.json";

    std::error_code ec;
    if (std::filesystem::exists(configPath, ec) && !ec) {
        std::cout << "ccc.config.json already exists: " << configPath.string() << "\n";
        return 1;
    }

    const std::string defaultConfig =
        "{\n"
        "  \"include\": [\n"
        "    \"AGENTS.md\",\n"
        "    \"docs\",\n"
        "    \"package.json\"\n"
        "  ],\n"
        "  \"exclude\": [\n"
        "    \"docs/internal-notes.md\"\n"
        "  ]\n"
        "}\n";

    try {
        ccc::writeTextFile(configPath, defaultConfig);
        std::cout << "Wrote default config to " << configPath.string() << "\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to write config: " << e.what() << "\n";
        return 2;
    }
}

} // namespace ccc