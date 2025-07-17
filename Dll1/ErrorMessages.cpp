// =============================================
// File: ErrorMessages.cpp
// Category: Error Formatting
// Purpose: Implements clean, professional error message formatting functions.
// =============================================
#include "ErrorMessages.h"
#include "ConfigParser.h"  // For LoaderConfig struct

namespace ErrorMessages {

    std::string formatHksNotFoundError(const std::string& hksPath, const LoaderConfig& config) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                        HKS FILE NOT FOUND                      \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE:\n"
            "  " + hksPath + "\n"
            "\n"
            "  ┌─ CONFIGURATION ────────────────────────────────────────────────┐\n"
            "   Relative Path: " + config.gameScriptPath.relativePath + "\n"
            "   Absolute Path: " + config.gameScriptPath.absolutePath + "\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  ┌─ TROUBLESHOOTING ──────────────────────────────────────────────┐\n"
            "   [1] Verify c0000.hks exists in your game directory\n"
            "   [2] Check if gameScriptPath points to correct folder\n"
            "   [3] Ensure you have read permissions for the target path\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  ┌─ CONFIGURATION EXAMPLES ───────────────────────────────────────┐\n"
            "   Vanilla Game: gameScriptPath = \"../game/script\"\n"
            "   Modded Game:  gameScriptPath = \"./mod/script\"\n"
            "   Full Path:    gameScriptPath = \"C:/Games/YourGame/script\"\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  Please update your configuration file and restart.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatHksAccessError(const std::string& hksPath, const std::string& errorDetails) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                         HKS ACCESS ERROR                       \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + hksPath + "\n"
            "  ERROR: " + errorDetails + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Verify file permissions and access rights\n"
            "   [2] Check if the target path exists and is accessible\n"
            "   [3] Run application as administrator if required\n"
            "   [4] Review gameScriptPath configuration in config file\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatHksSystemError(const std::string& hksPath) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                       SYSTEM ACCESS ERROR                      \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + hksPath + "\n"
            "  ERROR: Unknown system error occurred\n"
            "\n"
            "  ┌─ RECOMMENDED ACTIONS ──────────────────────────────────────────┐\n"
            "   [1] Verify gameScriptPath configuration is correct\n"
            "   [2] Check if target directory exists\n"
            "   [3] Run application with administrator privileges\n"
            "   [4] Restart the application and try again\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatHksWriteError(const std::string& hksPath, const std::string& errorDetails) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                         HKS WRITE ERROR                        \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + hksPath + "\n"
            "  ERROR: " + errorDetails + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Check if file is read-only or locked by another process\n"
            "   [2] Verify you have write permissions to the target directory\n"
            "   [3] Ensure sufficient disk space is available\n"
            "   [4] Run application as administrator if required\n"
            "   [5] Close any applications that might be using the file\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatHksWriteSystemError(const std::string& hksPath) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                      HKS WRITE SYSTEM ERROR                    \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + hksPath + "\n"
            "  ERROR: Unknown error occurred during file write operation\n"
            "\n"
            "  ┌─ RECOMMENDED ACTIONS ──────────────────────────────────────────┐\n"
            "   [1] Check disk space and file system health\n"
            "   [2] Verify file is not locked or in use by another process\n"
            "   [3] Run application with administrator privileges\n"
            "   [4] Try restarting the application and retry\n"
            "   [5] Check if antivirus software is blocking file access\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatHksReadError(const std::string& hksPath) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                          HKS READ ERROR                        \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + hksPath + "\n"
            "  ERROR: Unable to open file for reading\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Verify the file exists and is accessible\n"
            "   [2] Check file permissions (needs read access)\n"
            "   [3] Ensure file is not locked by another process\n"
            "   [4] Run application as administrator if required\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatEmptyGameScriptPathError(const std::string& configFile) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                    INVALID CONFIGURATION                       \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  ERROR: gameScriptPath is empty or invalid\n"
            "  CONFIG FILE: " + configFile + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Open your configuration file in a text editor\n"
            "   [2] Add or fix the gameScriptPath setting\n"
            "   [3] Ensure the path points to your game's script directory\n"
            "   [4] Save the configuration file and restart the application\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  ┌─ CONFIGURATION EXAMPLES ───────────────────────────────────────┐\n"
            "   gameScriptPath = \"../game/script\"\n"
            "   gameScriptPath = \"./mod/script\"\n"
            "   gameScriptPath = \"C:/Games/YourGame/script\"\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  Cannot proceed without valid gameScriptPath configuration.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatMe3ReadError(const std::string& me3Path, const std::string& errorDetails) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                         ME3 FILE READ ERROR                    \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + me3Path + "\n"
            "  ERROR: " + errorDetails + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Verify .me3 file exists and is accessible\n"
            "   [2] Check file permissions (needs read access)\n"
            "   [3] Ensure file is not corrupted or empty\n"
            "   [4] Run application as administrator if required\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatMe3WriteError(const std::string& me3Path, const std::string& errorDetails) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                        ME3 FILE WRITE ERROR                    \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET FILE: " + me3Path + "\n"
            "  ERROR: " + errorDetails + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Check if file is read-only or locked by another process\n"
            "   [2] Verify you have write permissions to the target directory\n"
            "   [3] Ensure sufficient disk space is available\n"
            "   [4] Run application as administrator if required\n"
            "   [5] Check if antivirus software is blocking file access\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatLuaSetupConfigError(const std::string& issue) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                    LUA SETUP CONFIGURATION ERROR               \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  ERROR: " + issue + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Check your configuration file for missing or invalid paths\n"
            "   [2] Ensure modulePath is properly configured\n"
            "   [3] Verify gameScriptPath points to a valid directory\n"
            "   [4] Check that paths contain valid characters and no whitespace\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  ┌─ CONFIGURATION EXAMPLES ───────────────────────────────────────┐\n"
            "   modulePath = \"mod/action/script/lua\"\n"
            "   gameScriptPath = \"mod/action/script\"\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  Cannot create Lua setup script without valid configuration.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatLuaSetupDirectoryError(const std::string& loaderDir, const std::string& errorDetails) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                  LUA SETUP DIRECTORY ERROR                     \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET DIRECTORY: " + loaderDir + "\n"
            "  ERROR: " + errorDetails + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Check if parent directory exists and is writable\n"
            "   [2] Verify you have permissions to create directories\n"
            "   [3] Ensure the path is not too long for the filesystem\n"
            "   [4] Run application as administrator if required\n"
            "   [5] Check if antivirus software is blocking directory creation\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatLuaSetupScriptWriteError(const std::string& setupScript, const std::string& errorDetails) {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                 LUA SETUP SCRIPT WRITE ERROR                   \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  TARGET SCRIPT: " + setupScript + "\n"
            "  ERROR: " + errorDetails + "\n"
            "\n"
            "  ┌─ RESOLUTION STEPS ─────────────────────────────────────────────┐\n"
            "   [1] Check if directory exists and is writable\n"
            "   [2] Verify sufficient disk space is available\n"
            "   [3] Ensure file is not locked by another process\n"
            "   [4] Run application as administrator if required\n"
            "   [5] Check if antivirus software is blocking file creation\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  The Lua module loader cannot function without this setup script.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }
}