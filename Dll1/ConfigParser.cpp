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
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// Helper function to trim whitespace from string
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Helper function to parse quoted strings more robustly
std::string parseQuotedValue(const std::string& value) {
    std::string trimmed = trim(value);
    if (trimmed.empty()) return "";

    // Handle quoted strings
    if (trimmed.size() >= 2) {
        char firstChar = trimmed.front();
        char lastChar = trimmed.back();

        // Check for matching quotes
        if ((firstChar == '"' && lastChar == '"') ||
            (firstChar == '\'' && lastChar == '\'')) {
            return trimmed.substr(1, trimmed.size() - 2);
        }
    }

    // Return unquoted value
    return trimmed;
}

// Helper function to parse boolean values
bool parseBoolValue(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return (lower == "true" || lower == "1" || lower == "yes" || lower == "on");
}

// Helper function to parse key-value pair from line
bool parseKeyValue(const std::string& line, std::string& key, std::string& value) {
    // Find the first '=' that's not inside quotes
    size_t eq = std::string::npos;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (!inQuotes && (c == '"' || c == '\'')) {
            inQuotes = true;
            quoteChar = c;
        }
        else if (inQuotes && c == quoteChar) {
            inQuotes = false;
            quoteChar = '\0';
        }
        else if (!inQuotes && c == '=') {
            eq = i;
            break;
        }
    }

    if (eq == std::string::npos) return false;

    key = trim(line.substr(0, eq));
    value = parseQuotedValue(line.substr(eq + 1));

    return !key.empty();
}

// Parse config path from .me3 file - moved from LuaLoader.cpp
std::string parseConfigPathFromMe3(const fs::path& me3Path) {
    std::ifstream me3in(me3Path);
    if (!me3in.is_open()) {
        return "";
    }

    std::string line;
    while (std::getline(me3in, line)) {
        // Remove comments
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Look for luaLoaderConfigPath
        auto pos = line.find("luaLoaderConfigPath");
        if (pos != std::string::npos) {
            auto eq = line.find('=', pos);
            if (eq != std::string::npos) {
                std::string value = line.substr(eq + 1);

                // Strip whitespace and quotes more robustly
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                // Remove quotes (both single and double)
                if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
                    value.erase(0, 1);
                }
                if (!value.empty() && (value.back() == '"' || value.back() == '\'')) {
                    value.pop_back();
                }

                if (!value.empty()) {
                    return value;
                }
            }
        }
    }
    return "";
}

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

    int configVersion = 1; // Default if not found
    std::string line;
    bool foundGameScriptPath = false;
    bool foundModulePath = false;
    int lineNumber = 0;

    while (std::getline(in, line)) {
        lineNumber++;

        // Remove inline comments (but not comments inside quoted strings)
        size_t commentPos = std::string::npos;
        bool inQuotes = false;
        char quoteChar = '\0';

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            if (!inQuotes && (c == '"' || c == '\'')) {
                inQuotes = true;
                quoteChar = c;
            }
            else if (inQuotes && c == quoteChar) {
                inQuotes = false;
                quoteChar = '\0';
            }
            else if (!inQuotes && c == '#') {
                commentPos = i;
                break;
            }
        }

        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        line = trim(line);

        // Skip empty lines and section headers
        if (line.empty() || line[0] == '[') continue;

        std::string key, value;
        if (!parseKeyValue(line, key, value)) {
            if (!isSilentMode()) {
                log("Warning: Invalid syntax on line " + std::to_string(lineNumber) + ": " + line);
            }
            continue;
        }

        //  Config version logic 
        if (key == "configVersion") {
            try {
                configVersion = std::stoi(value);
                if (configVersion < 1) {
                    log("Warning: configVersion must be >= 1, defaulting to 1", LOG_ERROR);
                    configVersion = 1;
                }
            }
            catch (const std::exception&) {
                log("Invalid configVersion value '" + value + "' on line " + std::to_string(lineNumber) + ". Defaulting to 1.", LOG_ERROR);
                configVersion = 1;
            }
            continue;
        }

        //  Path configurations 
        if (key == "scriptPath" || key == "gameScriptPath") {
            if (value.empty()) {
                log("Error: " + key + " cannot be empty on line " + std::to_string(lineNumber), LOG_ERROR);
                continue;
            }

            std::string absolutePath = resolvePathWithFallbacks(value, outConfig.configDir);
            outConfig.gameScriptPath = PathInfo(value, absolutePath, outConfig.configDir);
            foundGameScriptPath = true;

            if (!isSilentMode()) {
                log("Game Script Path (relative): " + value);
                log("Game Script Path (absolute): " + absolutePath);
            }
        }
        else if (key == "modulePath") {
            if (value.empty()) {
                log("Warning: modulePath is empty on line " + std::to_string(lineNumber) + ", will use gameScriptPath");
                continue;
            }

            std::string absolutePath = resolvePathWithFallbacks(value, outConfig.configDir);
            outConfig.modulePath = PathInfo(value, absolutePath, outConfig.configDir);
            foundModulePath = true;

            if (!isSilentMode()) {
                log("Module Path (relative): " + value);
                log("Module Path (absolute): " + absolutePath);
            }
        }

        //  Boolean configurations 
        else if (key == "silent") {
            outConfig.silentMode = parseBoolValue(value);
            setSilentMode(outConfig.silentMode);
            log("Silent mode: " + std::string(outConfig.silentMode ? "enabled" : "disabled"));
        }
        else if (key == "backupHKSonLaunch") {
            outConfig.backupHKSonLaunch = parseBoolValue(value);
            log("Backup HKS on launch: " + std::string(outConfig.backupHKSonLaunch ? "enabled" : "disabled"));
        }

        //  String configurations 
        else if (key == "backupHKSFolder") {
            outConfig.backupHKSFolder = value;
            log("Backup folder: " + (value.empty() ? "(same directory)" : value));
        }

        //  Unknown configuration
        else {
            if (!isSilentMode()) {
                log("Warning: Unknown configuration key '" + key + "' on line " + std::to_string(lineNumber));
            }
        }
    }

    // Config version validation (AFTER reading the whole file)
    if (configVersion < 1) {
        log("Config file is missing a valid configVersion. Please regenerate your config or update it manually.", LOG_ERROR);
        return false;
    }

    // Future version compatibility checks
    if (configVersion > 1) {
        log("Config file version " + std::to_string(configVersion) + " is newer than supported (1). Some features may not work correctly.", LOG_ERROR);
    }

    // Validate required fields
    if (!foundGameScriptPath) {
        log("Missing required gameScriptPath in config", LOG_ERROR);
        log("Add: gameScriptPath = \"relative/path/to/script\"", LOG_ERROR);
        return false;
    }

    // Set default modulePath if not specified
    if (!foundModulePath) {
        outConfig.modulePath = outConfig.gameScriptPath;
        if (!isSilentMode()) {
            log("No modulePath specified, using gameScriptPath: " + outConfig.modulePath.absolutePath);
        }
    }

    // Validate paths exist or can be created
    try {
        if (!fs::exists(outConfig.gameScriptPath.absolutePath)) {
            log("Warning: gameScriptPath does not exist: " + outConfig.gameScriptPath.absolutePath);
        }
        if (!fs::exists(outConfig.modulePath.absolutePath)) {
            log("Warning: modulePath does not exist: " + outConfig.modulePath.absolutePath);
        }
    }
    catch (const std::exception& e) {
        log("Warning: Cannot validate paths: " + std::string(e.what()));
    }

    log("Config parsed successfully with " + std::to_string(lineNumber) + " lines processed", LOG_OK);
    return true;
}