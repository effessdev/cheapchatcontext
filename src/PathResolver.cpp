#include "PathResolver.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace ccc {

    static void collectAllFilesFromNode(
        const FileNode& node,
        const std::string& currentPath,
        std::vector<std::string>& out)
    {
        std::string path = currentPath.empty()
            ? node.name
            : currentPath + "/" + node.name;

        if (!node.isDirectory) {
            out.push_back(path);
            return;
        }

        for (const auto& child : node.children) {
            collectAllFilesFromNode(child, path, out);
        }
    }

    const FileNode* PathResolver::findNode(
        const FileNode& node,
        const std::string& relPath,
        const std::string& currentPath) const
    {
        std::string fullPath = currentPath.empty()
            ? node.name
            : currentPath + "/" + node.name;

        if (fullPath == relPath)
            return &node;

        if (!node.isDirectory)
            return nullptr;

        for (const auto& child : node.children) {
            if (auto* res = findNode(child, relPath, fullPath))
                return res;
        }

        return nullptr;
    }

    PathResolver::PathResolver(
        const std::filesystem::path& root,
        const ccc::FileNode& rootNode)
        : root_(root), rootNode_(rootNode)
    {
        collectAllFilesFromNode(rootNode_, "", allFiles_);
    }

    bool PathResolver::isDir(const std::string& rel) const {
        const FileNode* node = findNode(rootNode_, rel);
        return node && node->isDirectory;
    }

    std::string PathResolver::resolveUnique(const std::string& input) const {
        std::vector<std::string> matches;

        for (const auto& f : allFiles_) {
            if (f.size() >= input.size() &&
                f.compare(f.size() - input.size(), input.size(), input) == 0) {
                matches.push_back(f);
            }
        }

        if (matches.empty()) {
            throw std::runtime_error("No match for: " + input);
        }

        if (matches.size() > 1) {
            std::string err = "Ambiguous path: " + input + "\nMatches:\n";
            for (const auto& m : matches)
                err += "  " + m + "\n";
            throw std::runtime_error(err);
        }

        return matches.front();
    }

    std::vector<std::string> PathResolver::resolveMany(const std::string& input) const {
        std::string resolved = resolveUnique(input);

        if (isDir(resolved)) {
            return expandDirectory(resolved);
        }

        return { resolved };
    }

    std::vector<std::string> PathResolver::expandDirectory(const std::string& relDir) const {
        std::vector<std::string> out;

        for (const auto& f : allFiles_) {
            if (f.rfind(relDir + "/", 0) == 0 || f == relDir) {
                out.push_back(f);
            }
        }

        return out;
    }

    void PathResolver::addIncludes(const std::vector<std::string>& inputs) {
        includeInputs_.insert(includeInputs_.end(), inputs.begin(), inputs.end());
    }

    void PathResolver::addExcludes(const std::vector<std::string>& inputs) {
        excludeInputs_.insert(excludeInputs_.end(), inputs.begin(), inputs.end());
    }

    std::vector<ResolvedFile> PathResolver::build() const {
        std::unordered_set<std::string> included;
        std::unordered_set<std::string> excluded;

        std::vector<ResolvedFile> result;

        auto markExclude = [&](const std::string& input) {
            try {
                auto files = resolveMany(input);
                for (const auto& f : files) {
                    excluded.insert(f);
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[ccc] exclude warning: " << e.what() << "\n";
            }
            };

        auto markInclude = [&](const std::string& input) {
            try {
                auto files = resolveMany(input);

                for (const auto& f : files) {
                    if (excluded.count(f)) continue;

                    if (!included.insert(f).second) {
                        std::cerr << "[ccc] warning: duplicate include ignored: " << f << "\n";
                        continue;
                    }

                    FileContent fc = readFileSafely(root_ / f);
                    result.push_back({ f, std::move(fc) });
                }
            }
            catch (const std::exception& e) {
                std::cerr << "[ccc] include warning: " << e.what() << "\n";
            }
            };

        for (const auto& i : includeInputs_)
            markInclude(i);

        for (const auto& e : excludeInputs_)
            markExclude(e);

        std::vector<ResolvedFile> finalOut;
        for (auto& f : result) {
            if (!excluded.count(f.relPath)) {
                finalOut.push_back(std::move(f));
            }
            else {
                std::cerr << "[ccc] excluded override removed: " << f.relPath << "\n";
            }
        }

        return finalOut;
    }

} // namespace ccc
