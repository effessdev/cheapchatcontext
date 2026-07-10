#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string readFileToStringCodeBlock(const fs::path &filePath);