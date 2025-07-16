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
    catch (...) {
        return path;
    }
}

std::string resolvePathWithFallbacks(const std::string& inputPath, const std::string& configDir) {
    if (inputPath.empty()) return inputPath;
    try {
        fs::path input(inputPath);
        // Strategy 1: If already absolute, normalize and return
        if (input.is_absolute()) {
            return normalizePath(input.string());
        }
        // Strategy 2: Resolve relative to config directory
        fs::path configPath(configDir);
        fs::path resolvedPath = configPath / input;
        resolvedPath = resolvedPath.lexically_normal();
        std::string candidate = normalizePath(resolvedPath.string());
        // Verify this path makes sense (optional validation)
        if (fs::exists(fs::path(candidate).parent_path()) || fs::exists(candidate)) {
            return candidate;
        }
        // Strategy 3: Try relative to current working directory
        fs::path cwdPath = fs::current_path() / input;
        cwdPath = cwdPath.lexically_normal();
        candidate = normalizePath(cwdPath.string());
        if (fs::exists(fs::path(candidate).parent_path()) || fs::exists(candidate)) {
            return candidate;
        }
        // Strategy 4: Try relative to executable directory
        char buf[MAX_PATH] = {};
        if (GetModuleFileNameA(nullptr, buf, MAX_PATH)) {
            fs::path exePath = fs::path(buf).parent_path() / input;
            exePath = exePath.lexically_normal();
            candidate = normalizePath(exePath.string());
            if (fs::exists(fs::path(candidate).parent_path()) || fs::exists(candidate)) {
                return candidate;
            }
        }
        // Fallback: Use config directory resolution even if parent doesn't exist
        return normalizePath((configPath / input).lexically_normal().string());
    }
    catch (...) {
        // Ultimate fallback: simple concatenation
        std::string result = configDir + "/" + inputPath;
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
}

std::vector<std::string> findConfigFiles(const fs::path& searchPath, int maxDepth) {
    std::vector<std::string> configFiles;
    if (maxDepth <= 0 || !fs::exists(searchPath)) return configFiles;
    try {
        for (const auto& entry : fs::directory_iterator(searchPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".me3") {
                configFiles.push_back(normalizePath(entry.path().string()));
                if (!isSilentMode()) {
                    log("Found config: " + entry.path().filename().string());
                }
            }
            else if (entry.is_directory() && maxDepth > 1) {
                auto subFiles = findConfigFiles(entry.path(), maxDepth - 1);
                configFiles.insert(configFiles.end(), subFiles.begin(), subFiles.end());
            }
        }
    }
    catch (...) {
        // Silently continue on filesystem errors
    }
    return configFiles;
}

// Enhanced validation with better error reporting - moved from LuaLoader.cpp
bool validatePaths(LoaderConfig& config) {
    bool allValid = true;

    // Validate gameScriptPath
    try {
        if (!fs::exists(config.gameScriptPath.absolutePath)) {
            log("Creating gameScriptPath directory: " + config.gameScriptPath.absolutePath);
            fs::create_directories(config.gameScriptPath.absolutePath);
        }

        if (!fs::is_directory(config.gameScriptPath.absolutePath)) {
            log("gameScriptPath is not a directory: " + config.gameScriptPath.absolutePath, LOG_ERROR);
            log("Relative path was: " + config.gameScriptPath.relativePath, LOG_ERROR);
            log("Resolved from config dir: " + config.configDir, LOG_ERROR);
            allValid = false;
        }
        else {
            log("Game script path validated", LOG_OK);
            if (!isSilentMode()) {
                log("  Relative: " + config.gameScriptPath.relativePath);
                log("  Absolute: " + config.gameScriptPath.absolutePath);
            }
        }
    }
    catch (const std::exception& e) {
        log("Cannot access gameScriptPath: " + std::string(e.what()), LOG_ERROR);
        log("Relative path: " + config.gameScriptPath.relativePath, LOG_ERROR);
        log("Absolute path: " + config.gameScriptPath.absolutePath, LOG_ERROR);
        allValid = false;
    }

    // Validate modulePath
    try {
        if (!fs::exists(config.modulePath.absolutePath)) {
            log("Creating modulePath directory: " + config.modulePath.absolutePath);
            fs::create_directories(config.modulePath.absolutePath);
        }

        if (!fs::is_directory(config.modulePath.absolutePath)) {
            log("modulePath is not a directory, using gameScriptPath", LOG_ERROR);
            config.modulePath = config.gameScriptPath;
        }
        else {
            log("Module path validated", LOG_OK);
            if (!isSilentMode()) {
                log("  Relative: " + config.modulePath.relativePath);
                log("  Absolute: " + config.modulePath.absolutePath);
            }
        }
    }
    catch (const std::exception& e) {
        log("Cannot access modulePath, using gameScriptPath: " + std::string(e.what()));
        config.modulePath = config.gameScriptPath;
    }

    return allValid;
}