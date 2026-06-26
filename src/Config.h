#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ccc {

// Reads `configPath` (expected schema below) and resolves it to a flat,
// de-duplicated list of file paths (forward-slash, relative to
// `rootDir`) that should be included in the generated context.
//
//   {
//     "include": [
//       "path/to/file.txt",
//       "path/to/a/folder"
//     ],
//     "exclude": [
//       "path/to/a/folder/skip_this.txt"
//     ]
//   }
//
// - Entries in "include" may be files or folders. Folders are expanded
//   recursively to every file inside them (this expansion deliberately
//   ignores .gitignore - an explicit include is an override).
// - Entries in "exclude" remove specific files (or whole sub-folders)
//   that would otherwise be pulled in by a folder include.
// - Missing/invalid entries are reported to stderr and skipped rather
//   than aborting the whole run.
//
// If `configPath` does not exist, returns an empty vector.
std::vector<std::string> resolveConfig(const std::filesystem::path& rootDir,
                                        const std::filesystem::path& configPath);

// Creates default config file in the given directory.
// Returns:
//  0 -> success
//  1 -> already exists
//  2 -> write error
int createDefaultConfig(const std::filesystem::path& rootDir);

}  // namespace ccc
