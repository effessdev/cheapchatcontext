#include "DirectoryScanner.h"

#include <algorithm>
#include <system_error>

namespace ccc {

namespace {

bool isDirEntryLess(const std::filesystem::directory_entry& a,
                     const std::filesystem::directory_entry& b) {
    const bool aDir = a.is_directory();
    const bool bDir = b.is_directory();
    if (aDir != bDir) {
        return aDir;  // directories sort before files
    }
    return a.path().filename().string() < b.path().filename().string();
}

void scanRecursive(const std::filesystem::path& absDir, const std::string& relDir,
                    GitignoreManager& gim, FileNode& node) {
    gim.enterDirectory(absDir, relDir);

    std::error_code ec;
    std::vector<std::filesystem::directory_entry> entries;
    for (auto it = std::filesystem::directory_iterator(absDir, ec);
         !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        entries.push_back(*it);
    }
    std::sort(entries.begin(), entries.end(), isDirEntryLess);

    for (const auto& entry : entries) {
        const std::string name = entry.path().filename().string();
        if (name == ".git") {
            continue;  // always skip version control metadata
        }

        std::error_code statEc;
        const bool isDir = entry.is_directory(statEc);
        if (statEc) {
            continue;
        }

        const std::string relPath = relDir.empty() ? name : relDir + "/" + name;
        if (gim.isIgnored(relPath, isDir)) {
            continue;
        }

        FileNode child;
        child.name = name;
        child.isDirectory = isDir;
        if (isDir) {
            scanRecursive(entry.path(), relPath, gim, child);
        }
        node.children.push_back(std::move(child));
    }

    gim.leaveDirectory();
}

}  // namespace

FileNode scanDirectory(const std::filesystem::path& rootAbsPath) {
    FileNode root;
    root.isDirectory = true;
    GitignoreManager gim;
    scanRecursive(rootAbsPath, "", gim, root);
    return root;
}

}  // namespace ccc
