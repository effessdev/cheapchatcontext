#pragma once

#include <string>

#include "DirectoryScanner.h"

namespace ccc {

// Renders `root`'s children as a classic ASCII directory tree, e.g.:
//
// my-project/
// |-- src/
// |   |-- main.cpp
// |   `-- utils.cpp
// |-- README.md
// `-- .gitignore
//
// `rootLabel` is printed as the top-level line.
std::string renderTree(const FileNode& root, const std::string& rootLabel);

}  // namespace ccc
