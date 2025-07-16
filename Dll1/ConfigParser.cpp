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

// Helper function to parse log level from string
LogLevel parseLogLevel(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "trace") return LOG_TRACE;
    if (lower == "debug") return LOG_DEBUG;
    if (lower == "info") return LOG_INFO;
    if (lower == "warning" || lower == "warn") return LOG_WARNING;
    if (lower == "error") return LOG_ERROR;

    // Default to info if unrecognized
    return LOG_INFO;
}

// Helper function to get log level name as string
std::string getLogLevelName(LogLevel level) {
    switch (level) {
    case LOG_TRACE: return "trace";
    case LOG_DEBUG: return "debug";
    case LOG_INFO: return "info";
    case LOG_WARNING: return "warning";
    case LOG_ERROR: return "error";
    case LOG_BRAND: return "brand";
    default: return "info";
    }
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

// Update config file to reset cleanup flag
bool updateCleanupFlag(const std::string& configPath, bool newValue) {
    if (!fs::exists(configPath)) {
        log("Config file not found for cleanup flag update: " + configPath, LOG_ERROR, "ConfigParser");
        return false;
    }

    // Read all lines
    std::ifstream in(configPath);
    if (!in.is_open()) {
        log("Failed to open config file for cleanup flag update", LOG_ERROR, "ConfigParser");
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    bool flagUpdated = false;
    int lineNumber = 0;

    while (std::getline(in, line)) {
        lineNumber++;

        // More flexible detection of the cleanup flag
        std::string trimmedLine = line;
        // Remove leading/trailing whitespace
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));
        trimmedLine.erase(trimmedLine.find_last_not_of(" \t") + 1);

        // Check if this line contains cleanupOnNextLaunch (not commented out)
        if (trimmedLine.find("cleanupOnNextLaunch") != std::string::npos &&
            trimmedLine.find('=') != std::string::npos &&
            trimmedLine.find('#') != 0) {  // Not a comment line (doesn't start with #)

            // Extract the part before the equals sign
            size_t eq = trimmedLine.find('=');
            std::string key = trimmedLine.substr(0, eq);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);

            // Verify it's actually the cleanup flag
            if (key == "cleanupOnNextLaunch") {
                // Replace the entire line with the new value
                lines.push_back("cleanupOnNextLaunch = " + std::string(newValue ? "true" : "false"));
                flagUpdated = true;
                log("Updated cleanupOnNextLaunch flag on line " + std::to_string(lineNumber), LOG_DEBUG, "ConfigParser");
            }
            else {
                lines.push_back(line);
            }
        }
        else {
            lines.push_back(line);
        }
    }
    in.close();

    if (!flagUpdated) {
        log("cleanupOnNextLaunch flag not found in config file", LOG_WARNING, "ConfigParser");
        log("Searching for any line containing 'cleanupOnNextLaunch':", LOG_DEBUG, "ConfigParser");

        // Debug: show lines that contain the text
        for (int i = 0; i < lines.size(); ++i) {
            if (lines[i].find("cleanupOnNextLaunch") != std::string::npos) {
                log("Line " + std::to_string(i + 1) + ": " + lines[i], LOG_DEBUG, "ConfigParser");
            }
        }

        // Add the flag if it doesn't exist
        log("Adding cleanupOnNextLaunch flag to config file", LOG_INFO, "ConfigParser");
        lines.push_back("");
        lines.push_back("# Added by cleanup system");
        lines.push_back("cleanupOnNextLaunch = " + std::string(newValue ? "true" : "false"));
        flagUpdated = true;
    }

    // Write back to file
    std::ofstream out(configPath);
    if (!out.is_open()) {
        log("Failed to write updated config file", LOG_ERROR, "ConfigParser");
        return false;
    }

    for (const auto& outputLine : lines) {
        out << outputLine << "\n";
    }
    out.close();

    log("Updated cleanupOnNextLaunch flag to: " + std::string(newValue ? "true" : "false"), LOG_INFO, "ConfigParser");
    return true;
}

bool parseTomlConfig(const std::string& tomlPath, LoaderConfig& outConfig) {
    // Store config directory for relative path resolution
    outConfig.configDir = normalizePath(fs::path(tomlPath).parent_path().string());
    outConfig.configFile = tomlPath;

    log("Config directory: " + outConfig.configDir, LOG_DEBUG, "ConfigParser");
    log("Parsing config: " + fs::path(tomlPath).filename().string(), LOG_DEBUG, "ConfigParser");

    std::ifstream in(tomlPath);
    if (!in.is_open()) {
        log("Failed to open config: " + fs::path(tomlPath).filename().string(), LOG_ERROR, "ConfigParser");
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
            log("Warning: Invalid syntax on line " + std::to_string(lineNumber) + ": " + line, LOG_WARNING, "ConfigParser");
            continue;
        }

        //  Config version logic 
        if (key == "configVersion") {
            try {
                configVersion = std::stoi(value);
                if (configVersion < 1) {
                    log("Warning: configVersion must be >= 1, defaulting to 1", LOG_WARNING, "ConfigParser");
                    configVersion = 1;
                }
            }
            catch (const std::exception&) {
                log("Invalid configVersion value '" + value + "' on line " + std::to_string(lineNumber) + ". Defaulting to 1.", LOG_ERROR, "ConfigParser");
                configVersion = 1;
            }
            continue;
        }

        //  Log level configuration
        else if (key == "logLevel") {
            LogLevel newLevel = parseLogLevel(value);
            setLogLevel(newLevel);
            log("Log level set to: " + getLogLevelName(newLevel), LOG_INFO, "ConfigParser");
        }

        //  Path configurations 
        else if (key == "scriptPath" || key == "gameScriptPath") {
            if (value.empty()) {
                log("Error: " + key + " cannot be empty on line " + std::to_string(lineNumber), LOG_ERROR, "ConfigParser");
                continue;
            }

            std::string absolutePath = resolvePathWithFallbacks(value, outConfig.configDir);
            outConfig.gameScriptPath = PathInfo(value, absolutePath, outConfig.configDir);
            foundGameScriptPath = true;

            log("Game Script Path (relative): " + value, LOG_DEBUG, "ConfigParser");
            log("Game Script Path (absolute): " + absolutePath, LOG_DEBUG, "ConfigParser");
        }
        else if (key == "modulePath") {
            if (value.empty()) {
                log("Warning: modulePath is empty on line " + std::to_string(lineNumber) + ", will use gameScriptPath", LOG_WARNING, "ConfigParser");
                continue;
            }

            std::string absolutePath = resolvePathWithFallbacks(value, outConfig.configDir);
            outConfig.modulePath = PathInfo(value, absolutePath, outConfig.configDir);
            foundModulePath = true;

            log("Module Path (relative): " + value, LOG_DEBUG, "ConfigParser");
            log("Module Path (absolute): " + absolutePath, LOG_DEBUG, "ConfigParser");
        }

        //  Boolean configurations 
        else if (key == "backupHKSonLaunch") {
            outConfig.backupHKSonLaunch = parseBoolValue(value);
            log("Backup HKS on launch: " + std::string(outConfig.backupHKSonLaunch ? "enabled" : "disabled"), LOG_INFO, "ConfigParser");
        }
        else if (key == "cleanupOnNextLaunch") {
            outConfig.cleanupOnNextLaunch = parseBoolValue(value);
            log("Cleanup on next launch: " + std::string(outConfig.cleanupOnNextLaunch ? "enabled" : "disabled"), LOG_INFO, "ConfigParser");
        }

        //  String configurations 
        else if (key == "backupHKSFolder") {
            outConfig.backupHKSFolder = value;
            log("Backup folder: " + (value.empty() ? "(same directory)" : value), LOG_INFO, "ConfigParser");
        }

        //  Unknown configuration
        else {
            log("Warning: Unknown configuration key '" + key + "' on line " + std::to_string(lineNumber), LOG_WARNING, "ConfigParser");
        }
    }

    // Config version validation (AFTER reading the whole file)
    if (configVersion < 1) {
        log("Config file is missing a valid configVersion. Please regenerate your config or update it manually.", LOG_ERROR, "ConfigParser");
        return false;
    }

    // Future version compatibility checks
    if (configVersion > 1) {
        log("Config file version " + std::to_string(configVersion) + " is newer than supported (1). Some features may not work correctly.", LOG_WARNING, "ConfigParser");
    }

    // Validate required fields
    if (!foundGameScriptPath) {
        log("Missing required gameScriptPath in config", LOG_ERROR, "ConfigParser");
        log("Add: gameScriptPath = \"relative/path/to/script\"", LOG_ERROR, "ConfigParser");
        return false;
    }

    // Set default modulePath if not specified
    if (!foundModulePath) {
        outConfig.modulePath = outConfig.gameScriptPath;
        log("No modulePath specified, using gameScriptPath: " + outConfig.modulePath.absolutePath, LOG_DEBUG, "ConfigParser");
    }

    // Validate paths exist or can be created
    try {
        if (!fs::exists(outConfig.gameScriptPath.absolutePath)) {
            log("Warning: gameScriptPath does not exist: " + outConfig.gameScriptPath.absolutePath, LOG_WARNING, "ConfigParser");
        }
        if (!fs::exists(outConfig.modulePath.absolutePath)) {
            log("Warning: modulePath does not exist: " + outConfig.modulePath.absolutePath, LOG_WARNING, "ConfigParser");
        }
    }
    catch (const std::exception& e) {
        log("Warning: Cannot validate paths: " + std::string(e.what()), LOG_WARNING, "ConfigParser");
    }

    log("Config parsed successfully with " + std::to_string(lineNumber) + " lines processed", LOG_INFO, "ConfigParser");
    return true;
}