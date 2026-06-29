std::vector<std::string> resolveConfig(const std::filesystem::path& rootDir,
    const std::filesystem::path& configPath) {
    (void)rootDir; // no longer used

    std::vector<std::string> includes;
    std::vector<std::string> excludes;

    std::error_code existsEc;
    if (!std::filesystem::exists(configPath, existsEc) || existsEc) {
        return {};
    }

    std::ifstream in(configPath, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Warning: could not open " << configPath.string() << "\n";
        return {};
    }

    nlohmann::json doc;
    try {
        in >> doc;
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Warning: failed to parse " << configPath.string()
            << ": " << e.what() << "\n";
        return {};
    }

    if (doc.contains("include") && doc["include"].is_array()) {
        for (const auto& v : doc["include"]) {
            if (v.is_string()) {
                includes.push_back(v.get<std::string>());
            }
        }
    }

    if (doc.contains("exclude") && doc["exclude"].is_array()) {
        for (const auto& v : doc["exclude"]) {
            if (v.is_string()) {
                excludes.push_back(v.get<std::string>());
            }
        }
    }

    return ConfigEntries{
        .includes = std::move(includes),
        .excludes = std::move(excludes)
    };
}