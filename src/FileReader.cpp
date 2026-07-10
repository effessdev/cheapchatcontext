#include <fstream>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include <regex>

namespace fs = std::filesystem;

std::string readFileToStringCodeBlock(const fs::path &filePath)
{
    // 1. Fast check via extension
    static const std::unordered_set<std::string> binaryExtensions = {
        ".exe", ".obj", ".o", ".lib", ".dll", ".so", ".a",
        ".png", ".jpg", ".jpeg", ".gif", ".pdf", ".zip"};

    if (binaryExtensions.count(filePath.extension().string()))
    {
        return "Skipped: Binary file extension detected.";
    }

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return "Error: Could not open file.";

    // 2. Fallback check: inspect content for null bytes
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string head;
    head.resize(std::min((std::streamsize)1024, size));
    file.read(&head[0], head.size());

    if (std::any_of(head.begin(), head.begin() + file.gcount(), [](char c)
                    { return c == '\0'; }))
    {
        return "Skipped: Binary content detected.";
    }

    // 3. Read full content and escape backticks
    file.seekg(0, std::ios::beg);
    std::string content(size, '\0');
    if (!file.read(&content[0], size))
        return "Error: Failed to read file.";

    // Escape ``` as \`\`\`
    content = std::regex_replace(content, std::regex("```"), "\\`\\`\\`");

    // Wrap in Markdown code block
    return "```\n" + content + "\n```";
}