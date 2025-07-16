// =============================================
// File: ConfigGenerator.cpp
// Category: Config Generation
// Purpose: Generates default LuaLoader TOML config files with instructions.
// =============================================
#include "ConfigGenerator.h"
#include "Logger.h"
#include <fstream>

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