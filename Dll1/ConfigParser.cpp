// =============================================
// File: ConfigParser.cpp
// Category: Config Parsing
// Purpose: Implements LoaderConfig parsing from .me3/TOML config files.
// =============================================
#include "ConfigParser.h"
#include "Logger.h"
#include "PathUtils.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

bool parseTomlConfig(const std::string& tomlPath, LoaderConfig& outConfig) {
    // Store config directory for relative path resolution
    outConfig.configDir = normalizePath(fs::path(tomlPath).parent_path().string());
    outConfig.configFile = tomlPath;

    if (!isSilentMode()) {
        log("Config directory: " + outConfig.configDir);
        log("Parsing config: " + fs::path(tomlPath).filename().string());
    }

    std::ifstream in(tomlPath);
    if (!in.is_open()) {
        log("Failed to open config: " + fs::path(tomlPath).filename().string(), LOG_ERROR);
        return false;
    }

    std::string line;
    bool foundGameScriptPath = false;
    bool foundModulePath = false;

    while (std::getline(in, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Skip empty lines, comments, and sections
        if (line.empty() || line[0] == '#' || line[0] == '[') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        // Trim key & value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        // Strip quotes
        if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
            char q = value.front();
            auto endQ = value.find(q, 1);
            if (endQ != std::string::npos) {
                value = value.substr(1, endQ - 1);
            }
        }

        if (key == "scriptPath" || key == "gameScriptPath") {
            std::string absolutePath = resolvePathWithFallbacks(value, outConfig.configDir);
            outConfig.gameScriptPath = PathInfo(value, absolutePath, outConfig.configDir);
            foundGameScriptPath = true;

            if (!isSilentMode()) {
                log("Game Script Path (relative): " + value);
                log("Game Script Path (absolute): " + absolutePath);
            }
        }
        else if (key == "modulePath") {
            std::string absolutePath = resolvePathWithFallbacks(value, outConfig.configDir);
            outConfig.modulePath = PathInfo(value, absolutePath, outConfig.configDir);
            foundModulePath = true;

            if (!isSilentMode()) {
                log("Module Path (relative): " + value);
                log("Module Path (absolute): " + absolutePath);
            }
        }
        else if (key == "silent" && (value == "true" || value == "1")) {
            outConfig.silentMode = true;
            setSilentMode(true);
            log("Silent mode enabled");
        }
        else if (key == "backupHKSonLaunch") {
            outConfig.backupHKSonLaunch = (value == "true" || value == "1");
            log("Backup HKS on launch: " + std::string(outConfig.backupHKSonLaunch ? "enabled" : "disabled"));
        }
        else if (key == "backupHKSFolder") {
            outConfig.backupHKSFolder = value;
            log("Backup folder: " + value);
        }
    }

    if (!foundGameScriptPath) {
        log("Missing gameScriptPath in config", LOG_ERROR);
        log("Add: gameScriptPath = \"relative/path/to/script\"", LOG_ERROR);
        return false;
    }

    if (!foundModulePath) {
        // Default: use gameScriptPath as modulePath
        outConfig.modulePath = outConfig.gameScriptPath;
        log("No modulePath specified, using gameScriptPath: " + outConfig.modulePath.absolutePath);
    }

    return true;
}