#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "GitIgnore.h"

namespace ccc {

// A single entry in the scanned project tree.
struct FileNode {
    std::string name;
    bool isDirectory = false;
    std::vector<FileNode> children;  // populated only when isDirectory == true
};

// Recursively scans `rootAbsPath`, skipping anything matched by an
// applicable .gitignore (root-level or nested) as well as the .git
// directory itself. The returned node represents the root directory; its
// `name` field is left empty (callers typically supply their own label).
FileNode scanDirectory(const std::filesystem::path& rootAbsPath);

}  // namespace ccc
