#include "GitIgnore.h"

#include <fstream>

namespace ccc {

namespace {

// Trims trailing spaces/tabs/CR (but not an escaped trailing space, which
// is a rare edge case we don't bother distinguishing - good enough for
// real-world .gitignore files).
std::string rtrim(const std::string& s) {
    std::size_t end = s.size();
    while (end > 0 && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r')) {
        --end;
    }
    return s.substr(0, end);
}

// Converts a single gitignore glob fragment (no leading/trailing slash,
// no leading '!') into the body of an ECMAScript regex that matches the
// equivalent forward-slash path fragment.
std::string globToRegexBody(const std::string& glob) {
    std::string re;
    const std::size_t n = glob.size();
    std::size_t i = 0;

    while (i < n) {
        const char c = glob[i];

        if (c == '*') {
            if (i + 1 < n && glob[i + 1] == '*') {
                const bool slashBefore = (i == 0) || glob[i - 1] == '/';
                const std::size_t after = i + 2;
                const bool slashAfter = (after < n) && glob[after] == '/';

                if (slashBefore && slashAfter) {
                    // "**/" -> matches zero or more whole directory segments.
                    re += "(?:.*/)?";
                    i = after + 1;
                } else if (slashBefore && after == n) {
                    // trailing "**" -> matches anything (including slashes).
                    re += ".*";
                    i = after;
                } else {
                    // "**" used in some other position - treat permissively.
                    re += ".*";
                    i = after;
                }
            } else {
                re += "[^/]*";
                ++i;
            }
        } else if (c == '?') {
            re += "[^/]";
            ++i;
        } else if (c == '[') {
            // Copy a character class through, translating it into a
            // (mostly) equivalent regex character class.
            std::string cls = "[";
            std::size_t j = i + 1;
            if (j < n && glob[j] == '!') {
                cls += '^';
                ++j;
            } else if (j < n && glob[j] == '^') {
                cls += "\\^";
                ++j;
            }
            while (j < n && glob[j] != ']') {
                const char cc = glob[j];
                if (cc == '\\' || cc == '^') {
                    cls += '\\';
                }
                cls += cc;
                ++j;
            }
            cls += ']';
            re += cls;
            i = (j < n) ? j + 1 : j;
        } else if (std::string("\\^$.|+(){}").find(c) != std::string::npos) {
            re += '\\';
            re += c;
            ++i;
        } else {
            re += c;
            ++i;
        }
    }

    return re;
}

}  // namespace

GitignoreRuleSet::GitignoreRuleSet(std::string baseDir) : baseDir_(std::move(baseDir)) {}

void GitignoreRuleSet::loadFromFile(const std::filesystem::path& gitignorePath) {
    std::ifstream in(gitignorePath);
    if (!in.is_open()) {
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        addPatternLine(line);
    }
}

void GitignoreRuleSet::addPatternLine(const std::string& rawLine) {
    std::string line = rtrim(rawLine);
    if (line.empty() || line[0] == '#') {
        return;
    }

    bool negation = false;
    if (line[0] == '!') {
        negation = true;
        line = line.substr(1);
    } else if (line.rfind("\\!", 0) == 0 || line.rfind("\\#", 0) == 0) {
        // Escaped leading '!' or '#' - treat literally.
        line = line.substr(1);
    }
    if (line.empty()) {
        return;
    }

    bool dirOnly = false;
    if (line.back() == '/') {
        dirOnly = true;
        line.pop_back();
    }
    if (line.empty()) {
        return;
    }

    const bool explicitlyAnchored = (line.front() == '/');
    if (explicitlyAnchored) {
        line = line.substr(1);
    }
    if (line.empty()) {
        return;
    }

    // Per git's rules: any pattern containing a slash anywhere (other than
    // a trailing one, already stripped above) is anchored to this
    // .gitignore's directory. A pattern with no slash at all may match at
    // any depth below that directory.
    const bool anchored = explicitlyAnchored || (line.find('/') != std::string::npos);

    const std::string body = globToRegexBody(line);
    const std::string fullRegex = (anchored ? "^" : "^(?:.*/)?") + body + "$";

    GitignoreRule rule;
    rule.isNegation = negation;
    rule.isDirOnly = dirOnly;
    rule.rawPattern = rawLine;
    try {
        rule.pattern = std::regex(fullRegex, std::regex::ECMAScript);
    } catch (const std::regex_error&) {
        // Malformed pattern (shouldn't normally happen) - skip it rather
        // than crashing the whole scan.
        return;
    }

    rules_.push_back(std::move(rule));
}

int GitignoreRuleSet::evaluate(const std::string& relPathFromRoot, bool isDirectory) const {
    if (rules_.empty()) {
        return 0;
    }

    // Compute the path relative to this rule set's base directory.
    std::string relToBase = relPathFromRoot;
    if (!baseDir_.empty()) {
        if (relToBase.rfind(baseDir_ + "/", 0) != 0) {
            // Not actually under this base - shouldn't happen given how
            // GitignoreManager maintains the stack, but be defensive.
            return 0;
        }
        relToBase = relToBase.substr(baseDir_.size() + 1);
    }
    if (relToBase.empty()) {
        return 0;
    }

    int result = 0;
    for (const GitignoreRule& rule : rules_) {
        if (rule.isDirOnly && !isDirectory) {
            continue;
        }
        if (std::regex_match(relToBase, rule.pattern)) {
            result = rule.isNegation ? -1 : 1;
        }
    }
    return result;
}

void GitignoreManager::enterDirectory(const std::filesystem::path& absoluteDirPath,
                                       const std::string& relDirPath) {
    GitignoreRuleSet ruleSet(relDirPath);
    const std::filesystem::path gitignorePath = absoluteDirPath / ".gitignore";
    std::error_code ec;
    if (std::filesystem::exists(gitignorePath, ec) && !ec) {
        ruleSet.loadFromFile(gitignorePath);
    }
    stack_.push_back(std::move(ruleSet));
}

void GitignoreManager::leaveDirectory() {
    if (!stack_.empty()) {
        stack_.pop_back();
    }
}

bool GitignoreManager::isIgnored(const std::string& relPath, bool isDirectory) const {
    // Evaluate every active rule set from the project root down to the
    // current directory. The *last* rule set with an opinion wins,
    // matching git's "more specific .gitignore overrides less specific
    // one" behavior. Within a rule set the last matching line wins too,
    // which is already handled by GitignoreRuleSet::evaluate.
    int decision = 0;
    for (const GitignoreRuleSet& ruleSet : stack_) {
        const int verdict = ruleSet.evaluate(relPath, isDirectory);
        if (verdict != 0) {
            decision = verdict;
        }
    }
    return decision > 0;
}

}  // namespace ccc
