#ifndef GITFILES_H
#define GITFILES_H

#include <filesystem>
#include <vector>
#include <string>

/// Returns a vector of the relative paths (forward-slash style) of all files tracked by Git in the given directory.
std::vector<std::string> getTrackedFiles(const std::filesystem::path &rootDir);

#endif // GITFILES_H
