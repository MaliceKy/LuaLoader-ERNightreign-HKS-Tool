// =============================================
// File: ConfigGenerator.cpp
// Category: Config Generation
// Purpose: Generates default LuaLoader TOML config files with instructions.
// =============================================
#include "ConfigGenerator.h"
#include "Logger.h"
#include <fstream>

void generateDefaultConfigToml(const std::string& configPath) {
    log("Generating default TOML config file", LOG_DEBUG, "ConfigGenerator");
    log("Target config path: " + configPath, LOG_DEBUG, "ConfigGenerator");

    std::ofstream out(configPath);
    if (!out.is_open()) {
        log("Failed to create default config file: " + configPath, LOG_ERROR, "ConfigGenerator");
        return;
    }

    out << R"(# ======================================
# LuaLoader Configuration (v1)
# Generated automatically by LuaLoader
# Author: Malice
# ======================================
configVersion = 1

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

# === LOGGING ===
# Logging verbosity. Set logLevel to one of:
#   trace   (everything, including super-verbose dev output)
#   debug   (debug, info, warnings, errors)
#   info    (normal user info, warnings, errors)  [default]
#   warning (only warnings and errors)
#   error   (only errors)
logLevel = "info"

# === HKS Backup Options ===
# backupHKSonLaunch behavior:
#   true  = Always backup c0000.hks on every launch
#   false = Only backup when actually injecting code (not when already injected)
backupHKSonLaunch = false        # true/false. If true, backup c0000.hks each launch. If false, only backup when injecting code.
backupHKSFolder = "HKS-Backups"  # Folder path for HKS backups (relative or absolute). Leave blank for same directory.

# === CLEANUP OPTIONS ===
# Set to true to remove all LuaLoader artifacts on next launch:
#   - Removes _module_loader directory
#   - Removes .modules_loaded flag files  
#   - Removes LuaLoader injection from c0000.hks (backed up to backupHKSFolder)
# This flag automatically resets to false after cleanup completes.
cleanupOnNextLaunch = false      # true/false. Set to true to cleanup and reset project state.

# ======================================
# --- INSTRUCTIONS ---
# Edit paths as needed, save this file, and relaunch the game.
# If you move this config, update the .me3 to point to it with 'luaLoaderConfigPath'.
# To cleanup the project: set cleanupOnNextLaunch = true and relaunch.
# ======================================
)";

    out.close();
    log("Default configuration file created successfully", LOG_INFO, "ConfigGenerator");
    log("Config file location: " + configPath, LOG_DEBUG, "ConfigGenerator");
}