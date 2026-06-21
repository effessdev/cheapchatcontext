#pragma once

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

namespace ccc {

// A single compiled pattern from a .gitignore file.
struct GitignoreRule {
    std::regex pattern;
    bool isNegation = false;  // pattern started with '!'
    bool isDirOnly = false;   // pattern ended with '/'
    std::string rawPattern;   // original text, kept for debugging
};

// All the rules contributed by one .gitignore file. `baseDir` is the
// directory containing that file, expressed as a forward-slash path
// relative to the project root ("" for the root .gitignore itself).
class GitignoreRuleSet {
public:
    explicit GitignoreRuleSet(std::string baseDir);

    // Parses every line of the given .gitignore file and stores the
    // resulting rules. Missing files are simply a no-op.
    void loadFromFile(const std::filesystem::path& gitignorePath);

    // Parses a single non-empty, non-comment gitignore line.
    void addPatternLine(const std::string& rawLine);

    // Checks `relPathFromRoot` (forward-slash, relative to the project
    // root) against every rule in this set, in file order. Returns:
    //   +1 -> the last matching rule says "ignore"
    //   -1 -> the last matching rule says "do not ignore" (negation)
    //    0 -> no rule in this set matched at all
    int evaluate(const std::string& relPathFromRoot, bool isDirectory) const;

    const std::string& baseDir() const { return baseDir_; }
    bool empty() const { return rules_.empty(); }

private:
    std::string baseDir_;
    std::vector<GitignoreRule> rules_;
};

// Walks alongside a directory traversal, maintaining the stack of
// .gitignore rule sets that apply at the current depth (root .gitignore,
// plus any in intervening directories) and answering ignore queries
// against all of them at once, with the usual git precedence: deeper /
// later rules override earlier ones.
class GitignoreManager {
public:
    // Call once when descending into `absoluteDirPath` (whose path
    // relative to the project root is `relDirPath`, "" for the root
    // itself). Loads that directory's .gitignore, if any, and pushes it
    // onto the stack.
    void enterDirectory(const std::filesystem::path& absoluteDirPath, const std::string& relDirPath);

    // Call once when finished with the directory most recently passed to
    // enterDirectory(), to pop its rules back off the stack.
    void leaveDirectory();

    // Determines whether `relPath` (forward-slash, relative to the
    // project root) should be ignored, given every currently active
    // .gitignore rule set (root-to-current-directory).
    bool isIgnored(const std::string& relPath, bool isDirectory) const;

private:
    std::vector<GitignoreRuleSet> stack_;
};

}  // namespace ccc
