#include "PathResolver.h"

#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace ccc {

    static std::vector<std::string> collectAllFiles(const fs::path& root) {
        std::vector<std::string> files;

        for (auto it = fs::recursive_directory_iterator(root); it != fs::recursive_directory_iterator(); ++it) {
            std::error_code ec;
            if (it->is_regular_file(ec) && !ec) {
                std::string rel = fs::relative(it->path(), root, ec).generic_string();
                if (!ec) files.push_back(rel);
            }
        }
        return files;
    }

    PathResolver::PathResolver(const fs::path& root) : root_(root) {
        allFiles_ = collectAllFiles(root_);
    }

    bool PathResolver::isDir(const std::string& rel) const {
        std::error_code ec;
        return fs::is_directory(root_ / rel, ec);
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
            for (const auto& m : matches) err += "  " + m + "\n";
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
                auto files = const_cast<PathResolver*>(this)->resolveMany(input);
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
                auto files = const_cast<PathResolver*>(this)->resolveMany(input);

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

        // 1. includes first
        for (const auto& i : includeInputs_) {
            markInclude(i);
        }

        // 2. excludes override EVERYTHING
        for (const auto& e : excludeInputs_) {
            markExclude(e);
        }

        // 3. re-filter result after excludes (critical correctness step)
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
