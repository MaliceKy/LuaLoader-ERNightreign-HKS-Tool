// =============================================
// File: PathUtils.cpp
// Category: Filesystem Utilities
// Purpose: Implements path normalization, fallback resolution logic, and path validation.
// =============================================
#include "PathUtils.h"
#include "ConfigParser.h"  // Include this to get LoaderConfig definition
#include "Logger.h"
#include <filesystem>
#include <algorithm>
#include <windows.h>

namespace fs = std::filesystem;

std::string normalizePath(const std::string& path) {
    if (path.empty()) return path;
    try {
        fs::path p(path);
        if (p.is_relative()) p = fs::absolute(p);
        p = p.lexically_normal();
        std::string result = p.string();
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
    catch (const std::exception& e) {
        log("Path normalization failed for '" + path + "': " + std::string(e.what()), LOG_TRACE, "PathUtils");
        return path;
    }
    catch (...) {
        log("Path normalization failed for '" + path + "': unknown error", LOG_TRACE, "PathUtils");
        return path;
    }
}

std::string resolvePathWithFallbacks(const std::string& inputPath, const std::string& configDir) {
    if (inputPath.empty()) return inputPath;

    log("Resolving path with fallbacks: " + inputPath, LOG_TRACE, "PathUtils");
    log("Config directory base: " + configDir, LOG_TRACE, "PathUtils");

    try {
        fs::path input(inputPath);

        // Strategy 1: If already absolute, normalize and return
        if (input.is_absolute()) {
            log("Path is already absolute", LOG_TRACE, "PathUtils");
            return normalizePath(input.string());
        }

        // Strategy 2: Resolve relative to config directory
        fs::path configPath(configDir);
        fs::path resolvedPath = configPath / input;
        resolvedPath = resolvedPath.lexically_normal();
        std::string candidate = normalizePath(resolvedPath.string());

        log("Trying config-relative path: " + candidate, LOG_TRACE, "PathUtils");

        // Verify this path makes sense (optional validation)
        if (fs::exists(fs::path(candidate).parent_path()) || fs::exists(candidate)) {
            log("Config-relative path exists, using: " + candidate, LOG_TRACE, "PathUtils");
            return candidate;
        }

        // Strategy 3: Try relative to current working directory
        fs::path cwdPath = fs::current_path() / input;
        cwdPath = cwdPath.lexically_normal();
        candidate = normalizePath(cwdPath.string());

        log("Trying CWD-relative path: " + candidate, LOG_TRACE, "PathUtils");

        if (fs::exists(fs::path(candidate).parent_path()) || fs::exists(candidate)) {
            log("CWD-relative path exists, using: " + candidate, LOG_TRACE, "PathUtils");
            return candidate;
        }

        // Strategy 4: Try relative to executable directory
        char buf[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, buf, MAX_PATH)) {
            fs::path exePath = fs::path(buf).parent_path() / input;
            exePath = exePath.lexically_normal();
            candidate = normalizePath(exePath.string());

            log("Trying executable-relative path: " + candidate, LOG_TRACE, "PathUtils");

            if (fs::exists(fs::path(candidate).parent_path()) || fs::exists(candidate)) {
                log("Executable-relative path exists, using: " + candidate, LOG_TRACE, "PathUtils");
                return candidate;
            }
        }

        // Fallback: Use config directory resolution even if parent doesn't exist
        std::string fallbackResult = normalizePath((configPath / input).lexically_normal().string());
        log("Using config-relative fallback: " + fallbackResult, LOG_TRACE, "PathUtils");
        return fallbackResult;
    }
    catch (const std::exception& e) {
        log("Path resolution failed: " + std::string(e.what()), LOG_WARNING, "PathUtils");
        // Ultimate fallback: simple concatenation
        std::string result = configDir + "/" + inputPath;
        std::replace(result.begin(), result.end(), '\\', '/');
        log("Using simple concatenation fallback: " + result, LOG_TRACE, "PathUtils");
        return result;
    }
    catch (...) {
        log("Path resolution failed: unknown error", LOG_WARNING, "PathUtils");
        // Ultimate fallback: simple concatenation
        std::string result = configDir + "/" + inputPath;
        std::replace(result.begin(), result.end(), '\\', '/');
        log("Using simple concatenation fallback: " + result, LOG_TRACE, "PathUtils");
        return result;
    }
}

std::vector<std::string> findConfigFiles(const fs::path& searchPath, int maxDepth) {
    std::vector<std::string> configFiles;
    if (maxDepth <= 0 || !fs::exists(searchPath)) {
        log("Skipping config search: invalid depth or path doesn't exist: " + searchPath.string(), LOG_TRACE, "PathUtils");
        return configFiles;
    }

    log("Searching for config files in: " + searchPath.string() + " (depth: " + std::to_string(maxDepth) + ")", LOG_TRACE, "PathUtils");

    try {
        for (const auto& entry : fs::directory_iterator(searchPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".me3") {
                configFiles.push_back(normalizePath(entry.path().string()));
                log("Found .me3 config file: " + entry.path().filename().string(), LOG_DEBUG, "PathUtils");
            }
            else if (entry.is_directory() && maxDepth > 1) {
                auto subFiles = findConfigFiles(entry.path(), maxDepth - 1);
                configFiles.insert(configFiles.end(), subFiles.begin(), subFiles.end());
            }
        }
    }
    catch (const std::exception& e) {
        log("Error searching directory '" + searchPath.string() + "': " + std::string(e.what()), LOG_TRACE, "PathUtils");
    }
    catch (...) {
        log("Error searching directory '" + searchPath.string() + "': unknown error", LOG_TRACE, "PathUtils");
    }

    return configFiles;
}

// Enhanced validation with better error reporting - moved from LuaLoader.cpp
bool validatePaths(LoaderConfig& config) {
    bool allValid = true;

    log("Validating configuration paths", LOG_DEBUG, "PathUtils");

    // Validate gameScriptPath
    try {
        log("Validating gameScriptPath: " + config.gameScriptPath.absolutePath, LOG_DEBUG, "PathUtils");

        if (!fs::exists(config.gameScriptPath.absolutePath)) {
            log("Creating gameScriptPath directory: " + config.gameScriptPath.absolutePath, LOG_DEBUG, "PathUtils");
            fs::create_directories(config.gameScriptPath.absolutePath);
        }

        if (!fs::is_directory(config.gameScriptPath.absolutePath)) {
            log("gameScriptPath is not a directory: " + config.gameScriptPath.absolutePath, LOG_ERROR, "PathUtils");
            log("Relative path was: " + config.gameScriptPath.relativePath, LOG_ERROR, "PathUtils");
            log("Resolved from config dir: " + config.configDir, LOG_ERROR, "PathUtils");
            allValid = false;
        }
        else {
            log("Game script path validated successfully", LOG_INFO, "PathUtils");
            log("  Relative: " + config.gameScriptPath.relativePath, LOG_DEBUG, "PathUtils");
            log("  Absolute: " + config.gameScriptPath.absolutePath, LOG_DEBUG, "PathUtils");
        }
    }
    catch (const std::exception& e) {
        log("Cannot access gameScriptPath: " + std::string(e.what()), LOG_ERROR, "PathUtils");
        log("Relative path: " + config.gameScriptPath.relativePath, LOG_ERROR, "PathUtils");
        log("Absolute path: " + config.gameScriptPath.absolutePath, LOG_ERROR, "PathUtils");
        allValid = false;
    }

    // Validate modulePath
    try {
        log("Validating modulePath: " + config.modulePath.absolutePath, LOG_DEBUG, "PathUtils");

        if (!fs::exists(config.modulePath.absolutePath)) {
            log("Creating modulePath directory: " + config.modulePath.absolutePath, LOG_DEBUG, "PathUtils");
            fs::create_directories(config.modulePath.absolutePath);
        }

        if (!fs::is_directory(config.modulePath.absolutePath)) {
            log("modulePath is not a directory, falling back to gameScriptPath", LOG_WARNING, "PathUtils");
            config.modulePath = config.gameScriptPath;
        }
        else {
            log("Module path validated successfully", LOG_INFO, "PathUtils");
            log("  Relative: " + config.modulePath.relativePath, LOG_DEBUG, "PathUtils");
            log("  Absolute: " + config.modulePath.absolutePath, LOG_DEBUG, "PathUtils");
        }
    }
    catch (const std::exception& e) {
        log("Cannot access modulePath, falling back to gameScriptPath: " + std::string(e.what()), LOG_WARNING, "PathUtils");
        config.modulePath = config.gameScriptPath;
    }

    log("Path validation complete. All paths valid: " + std::string(allValid ? "true" : "false"), LOG_DEBUG, "PathUtils");
    return allValid;
}