#include <filesystem>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

#include "FileFilter.h"

namespace fs = std::filesystem;

namespace
{
    std::string globToRegex(std::string_view glob)
    {
        std::string regex = "^";

        for (std::size_t i = 0; i < glob.size();)
        {
            // ** (recursive directories)
            if (glob[i] == '*' &&
                i + 1 < glob.size() &&
                glob[i + 1] == '*')
            {
                regex += ".*";
                i += 2;
                continue;
            }

            // * (within one directory)
            if (glob[i] == '*')
            {
                regex += "[^/]*";
                ++i;
                continue;
            }

            // Escape regex characters
            switch (glob[i])
            {
            case '.':
            case '+':
            case '(':
            case ')':
            case '^':
            case '$':
            case '|':
            case '{':
            case '}':
            case '[':
            case ']':
            case '\\':
            case '?':
                regex += '\\';
                break;
            }

            regex += glob[i];
            ++i;
        }

        regex += "$";
        return regex;
    }

    std::vector<std::regex> compilePatterns(
        const std::vector<std::string> &patterns)
    {
        std::vector<std::regex> compiled;
        compiled.reserve(patterns.size());

        for (const auto &pattern : patterns)
        {
            compiled.emplace_back(
                globToRegex(pattern),
                std::regex_constants::ECMAScript);
        }

        return compiled;
    }

    bool matchesAny(
        const std::string &path,
        const std::vector<std::regex> &patterns)
    {
        for (const auto &regex : patterns)
        {
            if (std::regex_match(path, regex))
                return true;
        }

        return false;
    }
}

std::vector<fs::path> filterFiles(
    const std::vector<fs::path> &files,
    const std::vector<std::string> &includePatterns,
    const std::vector<std::string> &excludePatterns)
{
    auto includes = compilePatterns(includePatterns);
    auto excludes = compilePatterns(excludePatterns);

    std::unordered_set<std::string> selected;

    // Include pass
    for (const auto &file : files)
    {
        std::string path = file.generic_string();

        if (matchesAny(path, includes))
            selected.insert(path);
    }

    // Exclude pass
    for (const auto &file : files)
    {
        std::string path = file.generic_string();

        if (selected.contains(path) &&
            matchesAny(path, excludes))
        {
            selected.erase(path);
        }
    }

    std::vector<fs::path> result;
    result.reserve(selected.size());

    for (const auto &file : files)
    {
        std::string path = file.generic_string();

        if (selected.contains(path))
            result.push_back(file);
    }

    return result;
}
