#include "Config.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace ccc {

namespace {

namespace fs = std::filesystem;

std::string toRelString(const fs::path& abs, const fs::path& root) {
    std::error_code ec;
    fs::path rel = fs::relative(abs, root, ec);
    if (ec) {
        return abs.generic_string();
    }
    return rel.generic_string();
}

// Recursively walks `absDir` (whose path relative to root is `relDir`),
// appending every file not blocked by `excludeSet` to `out`. Deliberately
// does NOT consult .gitignore - this is an explicit, user-requested
// inclusion and should win.
void walkIncludedFolder(const fs::path& absDir, const std::string& relDir,
                        const std::unordered_set<std::string>& excludeSet,
                        std::vector<std::string>& out, std::unordered_set<std::string>& seen) {
    std::error_code ec;
    std::vector<fs::directory_entry> entries;
    for (auto it = fs::directory_iterator(absDir, ec); !ec && it != fs::directory_iterator();
         it.increment(ec)) {
        entries.push_back(*it);
    }
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        const bool aDir = a.is_directory();
        const bool bDir = b.is_directory();
        if (aDir != bDir) return aDir;
        return a.path().filename().string() < b.path().filename().string();
    });

    for (const auto& entry : entries) {
        const std::string name = entry.path().filename().string();
        if (name == ".git") {
            continue;
        }
        const std::string relPath = relDir.empty() ? name : relDir + "/" + name;
        if (excludeSet.count(relPath) > 0) {
            continue;  // explicitly excluded file or whole sub-folder
        }

        std::error_code statEc;
        if (entry.is_directory(statEc)) {
            if (!statEc) {
                walkIncludedFolder(entry.path(), relPath, excludeSet, out, seen);
            }
        } else if (entry.is_regular_file(statEc)) {
            if (!statEc && seen.insert(relPath).second) {
                out.push_back(relPath);
            }
        }
    }
}

}  // namespace

std::vector<std::string> resolveConfig(const fs::path& rootDir, const fs::path& configPath) {
    std::vector<std::string> result;

    std::error_code existsEc;
    if (!fs::exists(configPath, existsEc) || existsEc) {
        return result;
    }

    std::ifstream in(configPath, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Warning: could not open " << configPath.string() << "\n";
        return result;
    }

    nlohmann::json doc;
    try {
        in >> doc;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Warning: failed to parse " << configPath.string() << ": " << e.what() << "\n";
        return result;
    }

    std::vector<std::string> includeEntries;
    std::unordered_set<std::string> excludeSet;

    if (doc.contains("include") && doc["include"].is_array()) {
        for (const auto& v : doc["include"]) {
            if (v.is_string()) {
                includeEntries.push_back(v.get<std::string>());
            }
        }
    }
    if (doc.contains("exclude") && doc["exclude"].is_array()) {
        for (const auto& v : doc["exclude"]) {
            if (v.is_string()) {
                // Normalize separators and strip any trailing slash so it
                // matches the relative paths we build during the walk.
                fs::path p(v.get<std::string>());
                std::string norm = p.generic_string();
                while (!norm.empty() && norm.back() == '/') {
                    norm.pop_back();
                }
                excludeSet.insert(norm);
            }
        }
    }

    std::unordered_set<std::string> seen;
    for (const std::string& entry : includeEntries) {
        fs::path target = fs::path(entry).is_absolute() ? fs::path(entry) : rootDir / entry;

        std::error_code statEc;
        if (!fs::exists(target, statEc) || statEc) {
            std::cerr << "Warning: include entry not found, skipping: " << entry << "\n";
            continue;
        }

        const std::string relRoot = toRelString(target, rootDir);
        if (excludeSet.count(relRoot) > 0) {
            continue;
        }

        if (fs::is_directory(target, statEc)) {
            walkIncludedFolder(target, relRoot, excludeSet, result, seen);
        } else if (fs::is_regular_file(target, statEc)) {
            if (seen.insert(relRoot).second) {
                result.push_back(relRoot);
            }
        }
    }

    return result;
}

}  // namespace ccc
