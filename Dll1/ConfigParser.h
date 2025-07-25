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

    // Cleanup settings
    bool cleanupOnNextLaunch = false;
};

// Main config parsing function
bool parseTomlConfig(const std::string& tomlPath, LoaderConfig& outConfig);

// Parse config path from .me3 file
std::string parseConfigPathFromMe3(const std::filesystem::path& me3Path);

// Updates the cleanupOnNextLaunch flag in the config file
// Used to reset the flag to false after cleanup completes
bool updateCleanupFlag(const std::string& configPath, bool newValue);


// Validates HKS file before backup
// Logs result and errors internall
bool validateHKSForBackup(const std::string& hksPath);