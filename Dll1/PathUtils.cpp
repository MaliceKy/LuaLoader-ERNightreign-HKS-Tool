// =============================================
// File: PathUtils.cpp
// Category: Filesystem Utilities
// Purpose: Implements path normalization and fallback resolution logic.
// =============================================
#include "PathUtils.h"
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