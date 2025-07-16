// =============================================
// File: Me3Utils.cpp
// Category: ME3 File Utilities
// Purpose: Utilities for manipulating .me3 files and injecting configuration paths.
// =============================================
#include "Me3Utils.h"
#include "Logger.h"
#include <fstream>
#include <vector>
#include <algorithm>

// Helper for case-insensitive search
std::string toLower(const std::string& str) {
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return res;
}

void injectTomlPathToMe3(const std::string& me3Path, const std::string& tomlPath) {
    log("Injecting TOML config path into .me3 file", LOG_DEBUG, "Me3Utils");
    log("Target .me3 file: " + me3Path, LOG_DEBUG, "Me3Utils");
    log("TOML config path to inject: " + tomlPath, LOG_DEBUG, "Me3Utils");

    std::ifstream in(me3Path);
    if (!in.is_open()) {
        log("Cannot open .me3 file for reading: " + me3Path, LOG_ERROR, "Me3Utils");
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool injected = false;
    bool foundExistingConfig = false;

    // Normalize the path to use forward slashes
    std::string normTomlPath = tomlPath;
    std::replace(normTomlPath.begin(), normTomlPath.end(), '\\', '/');
    log("Normalized path for injection: " + normTomlPath, LOG_TRACE, "Me3Utils");

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
            lines.push_back("# LuaLoader Toml Path Configuration (paths can be relative (to .me3 file) or absolute)");
            lines.push_back("luaLoaderConfigPath = \"" + normTomlPath + "\"");
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
        lines.push_back("luaLoaderConfigPath = \"" + normTomlPath + "\"");
        log("profileVersion line not found, appending config path at end of file", LOG_WARNING, "Me3Utils");
    }

    // Write the lines back to the file
    std::ofstream out(me3Path, std::ios::trunc);
    if (!out.is_open()) {
        log("Cannot open .me3 file for writing: " + me3Path, LOG_ERROR, "Me3Utils");
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
    log("Successfully modified .me3 file: " + me3Path, LOG_DEBUG, "Me3Utils");
}