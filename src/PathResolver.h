#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_set>

#include "FileUtils.h"

namespace ccc {

    struct ResolvedFile {
        std::string relPath;
        FileContent content;
    };

    class PathResolver {
    public:
        explicit PathResolver(const std::filesystem::path& root);

        // CLI inputs
        void addIncludes(const std::vector<std::string>& inputs);
        void addExcludes(const std::vector<std::string>& inputs);

        // Build final file list
        std::vector<ResolvedFile> build() const;

    private:
        std::filesystem::path root_;

        std::vector<std::string> allFiles_; // full relative paths from scan

        std::vector<std::string> includeInputs_;
        std::vector<std::string> excludeInputs_;

        std::string resolveUnique(const std::string& input) const;
        std::vector<std::string> resolveMany(const std::string& input) const;

        bool isDir(const std::string& rel) const;
        std::vector<std::string> expandDirectory(const std::string& relDir) const;
    };

} // namespace ccc
