// =============================================
// File: ConfigParser.h
// Category: Config Parsing
// Purpose: Declares LoaderConfig struct and config parsing function for .me3/TOML files.
// =============================================
#pragma once
#include <string>
#include <filesystem>

struct PathInfo {
    std::string relativePath;
    std::string absolutePath;
    std::string basePath;

    PathInfo() = default;
    PathInfo(const std::string& rel, const std::string& abs, const std::string& base)
        : relativePath(rel), absolutePath(abs), basePath(base) {
    }
};

struct LoaderConfig {
    PathInfo gameScriptPath;
    PathInfo modulePath;
    std::string configFile;
    std::string configDir;

    // Debug log settings
    bool silentMode = false;

    // Backing up HKS file
    bool backupHKSonLaunch = true;
    std::string backupHKSFolder;
};

bool parseTomlConfig(const std::string& tomlPath, LoaderConfig& outConfig);
std::string parseConfigPathFromMe3(const std::filesystem::path& me3Path);
