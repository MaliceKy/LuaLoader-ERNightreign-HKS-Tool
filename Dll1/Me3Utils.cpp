// =============================================
// File: Me3Utils.cpp
// Category: ME3 File Utilities
// Purpose: Utilities for manipulating .me3 files and injecting configuration paths.
// =============================================
#include "Me3Utils.h"
#include "Logger.h"
#include "ErrorMessages.h"  // ADDED: For beautiful error messages
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>  // For relative path conversion

namespace fs = std::filesystem;

// Helper for case-insensitive search
std::string toLower(const std::string& str) {
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return res;
}

// Convert absolute path to relative path for portability
std::string makePathRelative(const std::string& me3Path, const std::string& tomlPath) {
    try {
        fs::path me3Dir = fs::path(me3Path).parent_path();
        fs::path tomlFullPath = fs::absolute(tomlPath);
        fs::path relativePath = fs::relative(tomlFullPath, me3Dir);

        // Convert to forward slashes for consistency
        std::string result = relativePath.string();
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }
    catch (const std::exception& e) {
        log("Warning: Failed to create relative path, using filename only: " + std::string(e.what()), LOG_WARNING, "Me3Utils");
        // Fallback: just use the filename
        return fs::path(tomlPath).filename().string();
    }
}

void injectTomlPathToMe3(const std::string& me3Path, const std::string& tomlPath) {
    log("Injecting TOML config path into .me3 file", LOG_DEBUG, "Me3Utils");
    log("Target .me3 file: " + me3Path, LOG_DEBUG, "Me3Utils");
    log("TOML config path to inject: " + tomlPath, LOG_DEBUG, "Me3Utils");

    std::ifstream in(me3Path);
    if (!in.is_open()) {
        // CHANGED: Use beautiful error message instead of basic log
        log(ErrorMessages::formatMe3ReadError(me3Path, "Unable to open file for reading"), LOG_BRAND);
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool injected = false;
    bool foundExistingConfig = false;

    // Convert to relative path instead of just normalizing
    std::string pathToStore = makePathRelative(me3Path, tomlPath);
    log("Converted to relative path: " + pathToStore, LOG_DEBUG, "Me3Utils");

    // Read file and prepare to rewrite, skipping any existing luaLoaderConfigPath
    while (std::getline(in, line)) {
        if (line.find("luaLoaderConfigPath") != std::string::npos) {
            log("Found existing luaLoaderConfigPath, removing: " + line, LOG_DEBUG, "Me3Utils");
            foundExistingConfig = true;
            continue; // Remove any existing line
        }
        lines.push_back(line);

        // More robust search for profileVersion line
        std::string lowerLine = toLower(line);
        // Remove spaces and check if line starts with profileversion
        std::string trimmedLower = lowerLine;
        trimmedLower.erase(std::remove_if(trimmedLower.begin(), trimmedLower.end(), ::isspace), trimmedLower.end());

        if (!injected && (lowerLine.find("profileversion") != std::string::npos ||
            trimmedLower.find("profileversion=") == 0)) {
            lines.push_back("");
            lines.push_back("# LuaLoader Configuration (relative path for portability)");
            lines.push_back("luaLoaderConfigPath = \"" + pathToStore + "\"");
            lines.push_back("");
            injected = true;
            log("Found profileVersion line, injecting config path after it", LOG_DEBUG, "Me3Utils");
        }
    }
    in.close();

    // If file is empty or we didn't find profileVersion, append at end
    if (!injected) {
        lines.push_back("");
        lines.push_back("# --- Added by LuaLoader ---");
        lines.push_back("luaLoaderConfigPath = \"" + pathToStore + "\"");
        log("profileVersion line not found, appending config path at end of file", LOG_WARNING, "Me3Utils");
    }

    // Write the lines back to the file
    std::ofstream out(me3Path, std::ios::trunc);
    if (!out.is_open()) {
        // CHANGED: Use beautiful error message instead of basic log
        log(ErrorMessages::formatMe3WriteError(me3Path, "Unable to open file for writing"), LOG_BRAND);
        return;
    }
    for (const auto& l : lines) {
        out << l << "\n";
    }
    out.close();

    if (foundExistingConfig) {
        log("Updated existing luaLoaderConfigPath in .me3 file", LOG_INFO, "Me3Utils");
    }
    else {
        log("Added new luaLoaderConfigPath to .me3 file", LOG_INFO, "Me3Utils");
    }
    log("Successfully modified .me3 file with relative path: " + pathToStore, LOG_INFO, "Me3Utils");
}