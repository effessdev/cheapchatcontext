#pragma once

#include "GitFiles.h"
#include <fstream>
#include <regex>

namespace fs = std::filesystem;

namespace ccc
{

    struct Rule
    {
        std::regex rx;
        bool isNegative;
        bool isDirOnly;
        fs::path baseDir;
    };

    // Translates full gitignore spec into regex patterns
    std::string buildRegex(std::string pattern, bool &isDirOnly, bool &isAnchored)
    {
        if (pattern.empty())
            return "";

        // Trailing slash marks directory-only rules
        isDirOnly = (pattern.back() == '/');
        if (isDirOnly)
            pattern.pop_back();

        // Slashes elsewhere anchor the rule to the directory of the .gitignore
        isAnchored = false;
        size_t firstSlash = pattern.find('/');
        if (firstSlash != std::string::npos)
        {
            isAnchored = true;
            if (firstSlash == 0)
                pattern.erase(0, 1);
        }

        std::string rx = "";
        for (size_t i = 0; i < pattern.length(); ++i)
        {
            if (pattern.compare(i, 3, "/**") == 0 && i + 3 == pattern.length())
            {
                rx += "(/.*)?";
                i += 2;
            }
            else if (pattern.compare(i, 4, "/**/") == 0)
            {
                rx += "(/|/.*/)";
                i += 3;
            }
            else if (pattern.compare(i, 3, "**/") == 0 && i == 0)
            {
                rx += "(^|/)";
                i += 2;
            }
            else if (pattern[i] == '*')
            {
                rx += "[^/]*";
            }
            else if (pattern[i] == '?')
            {
                rx += "[^/]";
            }
            else if (std::string(".+()|^${}[]\\").find(pattern[i]) != std::string::npos)
            {
                rx += "\\";
                rx += pattern[i]; // Escape regex metacharacters
            }
            else
            {
                rx += pattern[i];
            }
        }

        // Wrap for proper path boundaries
        return isAnchored ? "^" + rx + "(/.*)?$" : "(^|/)" + rx + "(/.*)?$";
    }

    void traverse(const fs::path &currentDir, const fs::path &rootDir, std::vector<Rule> rules, std::vector<std::string> &output)
    {
        fs::path gitignorePath = currentDir / ".gitignore";

        // Parse local .gitignore to update the current rule chain
        if (fs::exists(gitignorePath))
        {
            std::ifstream file(gitignorePath);
            std::string line;
            while (std::getline(file, line))
            {
                // Trim whitespace
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);

                // Skip comments and blank lines
                if (line.empty() || line[0] == '#')
                    continue;

                // Handle Negations
                bool isNegative = (line[0] == '!');
                if (isNegative)
                    line.erase(0, 1);

                bool isDirOnly, isAnchored;
                std::string rxStr = buildRegex(line, isDirOnly, isAnchored);

                try
                {
                    rules.push_back({std::regex(rxStr), isNegative, isDirOnly, currentDir});
                }
                catch (...)
                { /* Skip malformed patterns safely */
                }
            }
        }

        for (const auto &entry : fs::directory_iterator(currentDir))
        {
            fs::path path = entry.path();
            if (path.filename() == ".git")
                continue;

            bool isDir = entry.is_directory();
            bool ignored = false;

            // Evaluate rules backwards (last matching rule dictates status)
            for (auto it = rules.rbegin(); it != rules.rend(); ++it)
            {
                if (it->isDirOnly && !isDir)
                    continue;

                std::string relToRule = fs::relative(path, it->baseDir).generic_string();
                if (std::regex_match(relToRule, it->rx))
                {
                    ignored = !it->isNegative;
                    break; // Last specified rule dictates fate
                }
            }

            if (ignored)
                continue;

            if (isDir)
            {
                traverse(path, rootDir, rules, output);
            }
            else
            {
                output.push_back(fs::relative(path, rootDir).generic_string());
            }
        }
    }

    std::vector<std::string> getTrackedFiles(const fs::path &rootDir)
    {
        std::vector<std::string> trackedFiles;
        if (fs::exists(rootDir) && fs::is_directory(rootDir))
        {
            traverse(rootDir, rootDir, {}, trackedFiles);
        }
        return trackedFiles;
    }

} // namespace ccc