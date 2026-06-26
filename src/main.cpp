#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "Clipboard.h"
#include "Config.h"
#include "DirectoryScanner.h"
#include "FileUtils.h"
#include "MarkdownBuilder.h"
#include "TreeRenderer.h"
#include "PathResolver.h"

namespace fs = std::filesystem;

namespace {

    void printHelp() {
        std::cout
            << "ccc - Copy Context to Clipboard\n\n"
            << "Scans the current directory (honoring .gitignore), reads AGENTS.md and any\n"
            << "files/folders listed in ccc.config.json, and copies a single Markdown\n"
            << "context block to the clipboard.\n\n"
            << "Usage: ccc [options]\n\n"
            << "Options:\n"
            << "  config     Create a default ccc.config.json in the current directory\n"
            << "  -a ...     Add files/folders (overrides .gitignore)\n"
            << "  -e ...     Exclude files/folders (highest priority)\n"
            << "  -h         Show this help message\n";
    }

} // namespace

int main(int argc, char** argv) {
    // e: argc: argument count (an int)
    // e: argv: argument vector (an array of C-style strings (char*))

    std::vector<std::string> addArgs;
    std::vector<std::string> excludeArgs;

    // -------------------------
    // Parse CLI
    // -------------------------
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        }

        else if (arg == "config") {
            return ccc::createDefaultConfig(fs::current_path());
        }

        else if (arg == "-a") {
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                addArgs.emplace_back(argv[++i]);
            }
        }

        else if (arg == "-e") {
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                excludeArgs.emplace_back(argv[++i]);
            }
        }

        else {
            std::cerr << "Unknown argument: " << arg << " (use --help)\n";
            return 1;
        }
    }

    // -------------------------
    // Root
    // -------------------------
    const fs::path root = fs::current_path();

    // -------------------------
    // Project tree (gitignore aware)
    // -------------------------
    const ccc::FileNode rootNode = ccc::scanDirectory(root);

    std::string rootLabel = root.filename().string();
    if (rootLabel.empty()) {
        rootLabel = root.generic_string();
    }

    const std::string projectStructure =
        ccc::renderTree(rootNode, rootLabel);

    // -------------------------
    // Config includes
    // -------------------------
    const fs::path configPath = root / "ccc.config.json";
    const std::vector<std::string> configIncludes =
        ccc::resolveConfig(root, configPath);

    // -------------------------
    // Resolve files (CLI + config)
    // -------------------------
    ccc::PathResolver resolver(root);

    resolver.addIncludes(configIncludes);
    resolver.addIncludes(addArgs);
    resolver.addExcludes(excludeArgs);

    std::vector<ccc::ResolvedFile> resolved;

    try {
        resolved = resolver.build();
    }
    catch (const std::exception& e) {
        std::cerr << "ccc: " << e.what() << "\n";
        return 1;
    }

    if (resolved.empty()) {
        std::cerr << "ccc: no files selected\n";
        return 1;
    }

    std::vector<std::pair<std::string, ccc::FileContent>> otherFiles;
    otherFiles.reserve(resolved.size());

    for (const auto& f : resolved) {
        otherFiles.emplace_back(f.relPath, f.content);
    }

    // -------------------------
    // Build context
    // -------------------------
    const std::string context =
        ccc::buildContext(projectStructure, otherFiles);

    // -------------------------
    // Clipboard ONLY (no fallback, no bypass)
    // -------------------------
    if (!ccc::setClipboardText(context)) {
        std::cerr << "Failed to copy to clipboard.\n";
#ifndef _WIN32
        std::cerr << "Install: wl-clipboard, xclip, or xsel\n";
#endif
        return 1;
    }

    std::cout
        << "Context copied to clipboard ("
        << context.size() << " bytes, "
        << otherFiles.size() << " file(s))\n";

    return 0;
}