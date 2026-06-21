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

    // 2. AGENTS.md, if present at the project root.
    std::optional<std::string> agentsContent;
    const fs::path agentsPath = root / "AGENTS.md";
    std::error_code existsEc;
    if (fs::exists(agentsPath, existsEc) && !existsEc && fs::is_regular_file(agentsPath)) {
        const ccc::FileContent fc = ccc::readFileSafely(agentsPath);
        if (fc.readError) {
            std::cerr << "Warning: found AGENTS.md but could not read it.\n";
        } else if (fc.isBinary) {
            std::cerr << "Warning: AGENTS.md looks like a binary file; skipping its contents.\n";
        } else {
            agentsContent = fc.content;
        }
    }

    // 3. ccc.config.json - explicit extra files/folders to include.
    const fs::path configPath = root / "ccc.config.json";
    const std::vector<std::string> otherRelPaths = ccc::resolveConfig(root, configPath);

    std::vector<std::pair<std::string, ccc::FileContent>> otherFiles;
    otherFiles.reserve(otherRelPaths.size());
    for (const std::string& rel : otherRelPaths) {
        const fs::path abs = root / rel;
        otherFiles.emplace_back(rel, ccc::readFileSafely(abs));
    }

    // 4. Assemble the final Markdown context.
    const std::string context = ccc::buildContext(projectStructure, agentsContent, otherFiles);

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
