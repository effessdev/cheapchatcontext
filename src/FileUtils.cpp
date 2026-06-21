#include "FileUtils.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace ccc {

FileContent readFileSafely(const std::filesystem::path& path) {
    FileContent result;

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        result.readError = true;
        return result;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (in.bad()) {
        result.readError = true;
        return result;
    }

    result.content = buffer.str();

    // Heuristic binary detection: a NUL byte essentially never appears in
    // genuine text source files, but is extremely common in binaries.
    // We only need to look at a prefix to keep this cheap on large files.
    const std::size_t scanLength = std::min<std::size_t>(result.content.size(), 8000);
    for (std::size_t i = 0; i < scanLength; ++i) {
        if (result.content[i] == '\0') {
            result.isBinary = true;
            break;
        }
    }

    return result;
}

void writeTextFile(const std::filesystem::path& path, const std::string& content) {
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

}  // namespace ccc
