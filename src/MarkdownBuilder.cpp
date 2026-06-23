#include "MarkdownBuilder.h"

#include <sstream>

namespace ccc {

namespace {

std::size_t longestBacktickRun(const std::string& content) {
    std::size_t longest = 0;
    std::size_t current = 0;
    for (char c : content) {
        if (c == '`') {
            ++current;
            longest = std::max(longest, current);
        } else {
            current = 0;
        }
    }
    return longest;
}

}  // namespace

std::string wrapInCodeFence(const std::string& content, const std::string& language) {
    // CommonMark requires the closing fence to be at least as long as the
    // opening one, and a fence can't be "broken" by a shorter run of
    // backticks inside the content. So instead of escaping characters
    // (which isn't really possible for backticks in Markdown), we just
    // make our fence longer than any backtick run already present.
    const std::size_t fenceLength = std::max<std::size_t>(3, longestBacktickRun(content) + 1);
    const std::string fence(fenceLength, '`');

    std::string body = content;
    if (!body.empty() && body.back() != '\n') {
        body += '\n';
    }

    std::ostringstream out;
    out << fence << language << "\n" << body << fence << "\n";
    return out.str();
}

std::string buildContext(const std::string& projectStructure,
                          const std::vector<std::pair<std::string, FileContent>>& otherFiles) {
    std::ostringstream out;

    out << "# Context:\n\n";

    out << "## Project structure:\n\n";
    out << wrapInCodeFence(projectStructure) << "\n";

    if (!otherFiles.empty()) {
        out << "## Other important files\n\n";
        for (const auto& [relPath, fc] : otherFiles) {
            out << "### " << relPath << "\n\n";
            if (fc.readError) {
                out << "*Could not read this file.*\n\n";
            } else if (fc.isBinary) {
                out << "*Binary file - contents omitted.*\n\n";
            } else {
                out << wrapInCodeFence(fc.content) << "\n";
            }
        }
    }

    out << "---\n\n\n";

    return out.str();
}

}  // namespace ccc
