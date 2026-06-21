#pragma once

#include <filesystem>
#include <string>

namespace ccc {

// Result of an attempt to read a file from disk.
struct FileContent {
    std::string content;
    bool isBinary = false;   // heuristically detected (NUL byte in first chunk)
    bool readError = false;  // could not open / read the file at all
};

// Reads a file fully into memory. Never throws; failures are reported via
// the returned struct's `readError` flag so callers can decide how to
// handle them (e.g. note it in the generated context instead of crashing).
FileContent readFileSafely(const std::filesystem::path& path);

// Writes `content` to `path`, overwriting any existing file. Creates parent
// directories if needed. Throws std::filesystem::filesystem_error on failure.
void writeTextFile(const std::filesystem::path& path, const std::string& content);

}  // namespace ccc
