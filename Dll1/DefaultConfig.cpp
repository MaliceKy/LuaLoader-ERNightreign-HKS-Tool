// =============================================
// File: DefaultConfig.cpp
// Category: Config Generation
// Purpose: Generates a default LuaLoader TOML config file with instructions.
// =============================================

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "Logger.h"

void generateDefaultConfigToml(const std::string& configPath) {
    std::ofstream out(configPath);
    if (!out.is_open()) {
        log("Failed to generate default config at: " + configPath, LOG_ERROR);
        return;
    }
    out << R"(# ======================================
# LuaLoader Configuration (v1)
# Generated automatically by LuaLoader
# Author: Malice
# ======================================
# configVersion = 1

# You can place this file anywhere and set the path in your .me3 file:
#   luaLoaderConfigPath = "D:/Path/To/LuaLoader.toml"
# If not set, this config is expected next to your .me3 file.

# === Path Validation Notes ===
# - All paths can be relative (to .me3 file) or absolute
# - Use forward slashes (/) or double backslashes (\\) in paths
# - Spaces in paths are supported

# REQUIRED: Path to your main HKS scripts
gameScriptPath = "mod/action/script"   # Relative to your .me3 file or absolute path

# OPTIONAL: Path to Lua modules (defaults to gameScriptPath)
modulePath = "mod/action/script/lua"

# Optional: Enable/disable silent mode (false = logging ON)
silent = false

# === HKS Backup Options ===
backupHKSonLaunch = true         # true/false. If true, backup c0000.hks each launch
backupHKSFolder = "HKS-Backups"  # Folder path for HKS backups (relative or absolute). Leave blank for same directory.

# ======================================
# --- INSTRUCTIONS ---
# Edit paths as needed, save this file, and relaunch the game.
# If you move this config, update the .me3 to point to it with 'luaLoaderConfigPath'.
# ======================================
)";
    out.close();
    log("Generated default LuaLoader config at: " + configPath, LOG_OK);
}

// Helper for case-insensitive search
std::string toLower(const std::string& str) {
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return res;
}

void injectTomlPathToMe3(const std::string& me3Path, const std::string& tomlPath) {
    std::ifstream in(me3Path);
    if (!in.is_open()) {
        log("Cannot open .me3 for injecting toml path.", LOG_ERROR);
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool injected = false;

    // Normalize the path to use forward slashes
    std::string normTomlPath = tomlPath;
    std::replace(normTomlPath.begin(), normTomlPath.end(), '\\', '/');

    // Read file and prepare to rewrite, skipping any existing luaLoaderConfigPath
    while (std::getline(in, line)) {
        if (line.find("luaLoaderConfigPath") != std::string::npos)
            continue; // Remove any existing line

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
            log("Found profileVersion line, injecting config path after it", LOG_OK);
        }
    }
    in.close();

    // If file is empty or we didn't find profileVersion, append at end
    if (!injected) {
        lines.push_back("");
        lines.push_back("# --- Added by LuaLoader ---");
        lines.push_back("luaLoaderConfigPath = \"" + normTomlPath + "\"");
        log("profileVersion line not found, appending config path at end", LOG_WARNING);
    }

    // Write the lines back to the file
    std::ofstream out(me3Path, std::ios::trunc);
    if (!out.is_open()) {
        log("Cannot open .me3 for writing toml path.", LOG_ERROR);
        return;
    }
    for (const auto& l : lines) out << l << "\n";
    out.close();

    log("Injected luaLoaderConfigPath into .me3: " + me3Path, LOG_OK);
}