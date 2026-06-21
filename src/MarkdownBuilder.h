#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "FileUtils.h"

namespace ccc {

// Wraps `content` in a Markdown fenced code block. If `content` itself
// contains a run of backticks, the fence is automatically widened (one
// longer than the longest run found) so the block can never be broken out
// of - this is the standard CommonMark way of "escaping" backticks inside
// a fenced code block, rather than altering the content itself.
std::string wrapInCodeFence(const std::string& content, const std::string& language = "");

// Assembles the final "# Context: ..." Markdown document from the three
// pieces gathered elsewhere in the program. Sections with no content
// (e.g. no AGENTS.md, or no files configured) are omitted.
std::string buildContext(const std::string& projectStructure,
                          const std::optional<std::string>& agentsContent,
                          const std::vector<std::pair<std::string, FileContent>>& otherFiles);

}  // namespace ccc
