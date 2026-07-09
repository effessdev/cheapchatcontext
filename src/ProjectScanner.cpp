#include "ProjectScanner.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace
{

    // -----------------------------------------------------------------------------
    // String Utilities
    // -----------------------------------------------------------------------------

    std::string TrimWhitespace(const std::string &str)
    {
        size_t start = 0;
        while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
        {
            start++;
        }
        size_t end = str.size();
        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        {
            end--;
        }
        return str.substr(start, end - start);
    }

    std::vector<std::string> SplitBySlash(const std::string &str)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, '/'))
        {
            if (!item.empty())
            {
                tokens.push_back(item);
            }
        }
        return tokens;
    }

    // -----------------------------------------------------------------------------
    // Glob Matching Logic
    // -----------------------------------------------------------------------------

    // O(N) iterative glob matching for standard wildcards ('*', '?') within a single path token.
    // Note: This matches component-level strings, so '*' inherently does not match across slashes.
    bool StringGlobMatch(const char *pattern, const char *str)
    {
        const char *p = pattern;
        const char *s = str;
        const char *star_p = nullptr;
        const char *star_s = nullptr;

        while (*s != '\0')
        {
            if (*p == *s || *p == '?')
            {
                p++;
                s++;
            }
            else if (*p == '*')
            {
                star_p = p++;
                star_s = s;
            }
            else if (star_p != nullptr)
            {
                p = star_p + 1;
                s = ++star_s;
            }
            else
            {
                return false;
            }
        }
        while (*p == '*')
        {
            p++;
        }
        return *p == '\0';
    }

    // Recursively matches a sequence of pattern tokens against a sequence of path tokens.
    // Handles "**" directory crossing behavior.
    bool MatchTokens(const std::vector<std::string> &pat, size_t p_idx,
                     const std::vector<std::string> &path, size_t path_idx)
    {
        // Base cases
        if (p_idx == pat.size() && path_idx == path.size())
            return true;
        if (p_idx == pat.size())
            return false;

        // "**" wildcard logic
        if (pat[p_idx] == "**")
        {
            if (path_idx == path.size())
            {
                // "**" can match zero tokens at the end
                return MatchTokens(pat, p_idx + 1, path, path_idx);
            }
            // Branch 1: "**" matches zero directory levels (move pattern pointer)
            if (MatchTokens(pat, p_idx + 1, path, path_idx))
                return true;

            // Branch 2: "**" matches current directory level (move path pointer, retain "**")
            if (MatchTokens(pat, p_idx, path, path_idx + 1))
                return true;

            return false;
        }

        if (path_idx == path.size())
            return false;

        // Regular glob token match
        if (StringGlobMatch(pat[p_idx].c_str(), path[path_idx].c_str()))
        {
            return MatchTokens(pat, p_idx + 1, path, path_idx + 1);
        }

        return false;
    }

    // -----------------------------------------------------------------------------
    // Ignore Rule Processing
    // -----------------------------------------------------------------------------

    struct IgnoreRule
    {
        std::vector<std::string> pattern_tokens;
        bool is_negated = false;
        bool is_dir_only = false;
        size_t base_depth = 0; // The directory depth where this rule was defined
    };

    void ParseGitIgnore(const std::filesystem::path &file_path, size_t current_depth, std::vector<IgnoreRule> &rules)
    {
        std::ifstream file(file_path);
        if (!file)
            return;

        std::string line;
        while (std::getline(file, line))
        {
            line = TrimWhitespace(line);

            // Skip blank lines and comments
            if (line.empty() || line[0] == '#')
                continue;

            IgnoreRule rule;
            rule.base_depth = current_depth;

            // Negation
            if (line[0] == '!')
            {
                rule.is_negated = true;
                line = TrimWhitespace(line.substr(1));
                if (line.empty())
                    continue;
            }

            // Directory only
            if (line.back() == '/')
            {
                rule.is_dir_only = true;
                line.pop_back();
                if (line.empty())
                    continue;
            }

            // Determine if pattern is anchored to the current .gitignore directory
            bool is_anchored = false;
            if (line[0] == '/')
            {
                is_anchored = true;
                line = line.substr(1);
            }
            else if (line.find('/') != std::string::npos)
            {
                is_anchored = true;
            }

            rule.pattern_tokens = SplitBySlash(line);

            // If unanchored, prefix with "**" so it can match anywhere downstream
            if (!is_anchored)
            {
                rule.pattern_tokens.insert(rule.pattern_tokens.begin(), "**");
            }

            rules.push_back(std::move(rule));
        }
    }

    bool IsIgnored(const std::vector<IgnoreRule> &rules,
                   const std::vector<std::string> &path_tokens,
                   bool is_dir)
    {
        bool ignored = false;

        for (const auto &rule : rules)
        {
            if (rule.is_dir_only && !is_dir)
                continue;
            if (path_tokens.size() < rule.base_depth)
                continue;

            // Compute relative path from the directory that defined the rule
            std::vector<std::string> rel_path(path_tokens.begin() + rule.base_depth, path_tokens.end());
            if (rel_path.empty())
                continue;

            if (MatchTokens(rule.pattern_tokens, 0, rel_path, 0))
            {
                // Last matching rule overrides previous matches
                ignored = !rule.is_negated;
            }
        }

        return ignored;
    }

    // -----------------------------------------------------------------------------
    // Directory Walker
    // -----------------------------------------------------------------------------

    void WalkDirectoryTree(
        const std::filesystem::path &current_dir,
        std::vector<std::string> &current_path_tokens,
        std::vector<IgnoreRule> &active_rules,
        std::vector<std::filesystem::path> &out_files)
    {
        // Save the rule count to restore state safely after descending
        const size_t initial_rule_count = active_rules.size();

        std::filesystem::path gitignore_path = current_dir / ".gitignore";
        std::error_code ec;
        if (std::filesystem::is_regular_file(gitignore_path, ec))
        {
            ParseGitIgnore(gitignore_path, current_path_tokens.size(), active_rules);
        }

        auto iterator = std::filesystem::directory_iterator(current_dir, ec);
        if (ec)
        {
            // Unreadable directory; safely return to continue with others
            active_rules.resize(initial_rule_count);
            return;
        }

        for (const auto &entry : iterator)
        {
            const std::string name = entry.path().filename().string();

            // Always skip `.git` entirely
            if (entry.is_directory(ec) && name == ".git")
                continue;

            // Skip hidden directories (starting with '.') as requested by "Only scan visible directories"
            if (entry.is_directory(ec) && !name.empty() && name[0] == '.')
                continue;

            current_path_tokens.push_back(name);

            const bool is_dir = entry.is_directory(ec);
            const bool is_link = entry.is_symlink(ec);

            // Apply rules logic
            if (!IsIgnored(active_rules, current_path_tokens, is_dir))
            {
                if (is_dir && !is_link)
                {
                    // Regular directory, recurse into it
                    WalkDirectoryTree(entry.path(), current_path_tokens, active_rules, out_files);
                }
                else
                {
                    // Build cross-platform relative path internally mapping tokens
                    std::filesystem::path rel_path;
                    for (const auto &token : current_path_tokens)
                    {
                        rel_path /= token;
                    }
                    out_files.push_back(std::move(rel_path));
                }
            }

            // Pop for the next iteration step at this depth
            current_path_tokens.pop_back();
        }

        // Clean up contextual rules appended at this hierarchy depth
        active_rules.resize(initial_rule_count);
    }
} // anonymous namespace

// -----------------------------------------------------------------------------
// Public API implementation
// -----------------------------------------------------------------------------

std::vector<std::filesystem::path> ScanProject(const std::filesystem::path &root)
{
    try
    {
        std::error_code ec;
        if (!std::filesystem::exists(root, ec) || ec)
        {
            throw std::runtime_error("Root directory does not exist: " + root.string());
        }
        if (!std::filesystem::is_directory(root, ec) || ec)
        {
            throw std::runtime_error("Root path is not a directory: " + root.string());
        }

        std::vector<std::filesystem::path> files;
        std::vector<std::string> path_tokens;
        std::vector<IgnoreRule> active_rules;

        // Efficient recursion logic handles relative pathing natively and builds `result.files`
        WalkDirectoryTree(root, path_tokens, active_rules, files);

        // Lexicographically sort by native path comparison
        std::sort(files.begin(), files.end());

        return files;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("Fatal error during project scan: ") + e.what());
    }
}
