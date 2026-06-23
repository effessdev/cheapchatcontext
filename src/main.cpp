#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Clipboard.h"
#include "Config.h"
#include "DirectoryScanner.h"
#include "FileUtils.h"
#include "MarkdownBuilder.h"
#include "TreeRenderer.h"

namespace fs = std::filesystem;

namespace {

void printHelp() {
    std::cout
        << "ccc - Copy Context to Clipboard\n\n"
        << "Scans the current directory (honoring .gitignore), reads AGENTS.md and any\n"
    << "files/folders listed in ccc.config.json, and copies a single Markdown\n"
        << "context block to the clipboard so you can paste it straight into a chatbot.\n\n"
        << "Usage: ccc [options]\n\n"
        << "Options:\n"
    << "  config                Create a default ccc.config.json in the current directory\n"
        << "  -o, --output <file>   Also write the generated context to <file>\n"
        << "      --no-clipboard    Don't touch the clipboard (useful with --output)\n"
        << "  -h, --help            Show this help message\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string outputFile;
    bool noClipboard = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "config") {
            // Create a default config file (ccc.config.json) in the current
            // working directory. Do not overwrite an existing file.
            const fs::path configPath = fs::current_path() / "ccc.config.json";
            std::error_code existsEc;
            if (fs::exists(configPath, existsEc) && !existsEc) {
                std::cout << "ccc.config.json already exists: " << configPath.string() << "\n";
                return 0;
            }
            const std::string defaultConfig =
                "{\n"
                "  \"include\": [\n"
                "    \"AGENTS.md\",\n"
                "    \"docs\",\n"
                "    \"package.json\"\n"
                "  ],\n"
                "  \"exclude\": [\n"
                "    \"docs/internal-notes.md\"\n"
                "  ]\n"
                "}\n";
            try {
                ccc::writeTextFile(configPath, defaultConfig);
                std::cout << "Wrote default config to " << configPath.string() << "\n";
                return 0;
            } catch (const std::exception& e) {
                std::cerr << "Failed to write config: " << e.what() << "\n";
                return 1;
            }
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "--no-clipboard") {
            noClipboard = true;
        } else {
            std::cerr << "Unknown argument: " << arg << " (use --help for usage)\n";
            return 1;
        }
    }

    // Always operate on the directory the tool was *invoked* from, not
    // wherever the executable itself lives - this is what lets people add
    // the exe's folder to PATH and run `ccc` from any project.
    const fs::path root = fs::current_path();

    // 1. Scan the project tree, honoring every .gitignore found along the
    //    way (root-level and nested).
    const ccc::FileNode rootNode = ccc::scanDirectory(root);
    std::string rootLabel = root.filename().string();
    if (rootLabel.empty()) {
        rootLabel = root.generic_string();
    }
    const std::string projectStructure = ccc::renderTree(rootNode, rootLabel);

    // 2. ccc.config.json - explicit extra files/folders to include.
    const fs::path configPath = root / "ccc.config.json";
    const std::vector<std::string> otherRelPaths = ccc::resolveConfig(root, configPath);

    std::vector<std::pair<std::string, ccc::FileContent>> otherFiles;
    otherFiles.reserve(otherRelPaths.size());
    for (const std::string& rel : otherRelPaths) {
        const fs::path abs = root / rel;
        otherFiles.emplace_back(rel, ccc::readFileSafely(abs));
    }

    // 4. Assemble the final Markdown context.
    const std::string context = ccc::buildContext(projectStructure, otherFiles);

    // 5. Output.
    if (!outputFile.empty()) {
        try {
            ccc::writeTextFile(outputFile, context);
            std::cout << "Wrote context to " << outputFile << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Failed to write " << outputFile << ": " << e.what() << "\n";
        }
    }

    if (!noClipboard) {
        if (ccc::setClipboardText(context)) {
            std::cout << "Context copied to clipboard (" << context.size() << " bytes, "
                       << otherFiles.size() << " extra file(s) included). Ready to paste!\n";
        } else {
            std::cerr << "Failed to copy to clipboard.\n";
#ifndef _WIN32
            std::cerr << "On Linux, install one of: wl-clipboard, xclip, or xsel.\n";
#endif
            return 1;
        }
    }

    return 0;
}
