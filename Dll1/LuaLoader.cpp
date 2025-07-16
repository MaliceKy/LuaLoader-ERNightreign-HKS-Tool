// =============================================
// File: LuaLoader.cpp
// Category: Main Loader Orchestration
// Purpose: Orchestrates DLL lifecycle and module loading logic (entry point).
// =============================================
#include "Console.h"
#include "Logger.h"
#include "PathUtils.h"
#include "ConfigParser.h"
#include "FlagFile.h"
#include "LuaSetup.h"
#include "HksInjector.h"
#include <windows.h>
#include <cstdlib> // for atexit
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

static LoaderConfig g_config;
static HMODULE g_hModule = nullptr;

// Clean up flag file on exit
void cleanup() {
    cleanupFlagFile(g_config.modulePath.absolutePath);
}

// Initialize paths from TOML configuration
static bool initializePaths() {
    char buf[MAX_PATH] = {};
    if (!GetModuleFileNameA(g_hModule, buf, MAX_PATH)) {
        log("Failed to get DLL path", LOG_ERROR);
        return false;
    }

    fs::path dllPath(buf);
    std::string basePath = normalizePath(dllPath.parent_path().string());

    if (!isSilentMode()) {
        log("DLL location: " + basePath);
        log("Searching for .me3 config files...");
    }

    // Enhanced search paths - look in multiple locations
    std::vector<fs::path> searchPaths = {
        dllPath.parent_path(),                          // Same directory as DLL
        dllPath.parent_path().parent_path(),            // Parent directory
        dllPath.parent_path().parent_path().parent_path(), // Grandparent directory
        fs::current_path(),                             // Current working directory
        fs::path("C:/"),                                // Root drive (last resort)
    };

    std::vector<std::string> configFiles;
    for (const auto& searchPath : searchPaths) {
        if (!isSilentMode()) {
            log("Searching: " + normalizePath(searchPath.string()));
        }

        auto found = findConfigFiles(searchPath, 2);
        configFiles.insert(configFiles.end(), found.begin(), found.end());

        // Stop searching if we found configs
        if (!configFiles.empty()) break;
    }

    if (configFiles.empty()) {
        log("No .me3 configuration file found!", LOG_ERROR);
        log("Create a .me3 file with gameScriptPath and modulePath", LOG_ERROR);
        log("Example:", LOG_ERROR);
        log("  gameScriptPath = \"mod/action/script\"", LOG_ERROR);
        log("  modulePath = \"mod/action/script/lua\"", LOG_ERROR);
        return false;
    }

    // Try to parse the first valid config file
    for (const auto& configFile : configFiles) {
        if (!isSilentMode()) {
            log("Trying config: " + fs::path(configFile).filename().string());
        }

        if (parseTomlConfig(configFile, g_config)) {
            log("Config loaded: " + fs::path(configFile).filename().string(), LOG_OK);
            return true;
        }
    }

    log("No valid configuration found", LOG_ERROR);
    return false;
}

// Enhanced validation with better error reporting
static bool validatePaths() {
    bool allValid = true;

    // Validate gameScriptPath
    try {
        if (!fs::exists(g_config.gameScriptPath.absolutePath)) {
            log("Creating gameScriptPath directory: " + g_config.gameScriptPath.absolutePath);
            fs::create_directories(g_config.gameScriptPath.absolutePath);
        }

        if (!fs::is_directory(g_config.gameScriptPath.absolutePath)) {
            log("gameScriptPath is not a directory: " + g_config.gameScriptPath.absolutePath, LOG_ERROR);
            log("Relative path was: " + g_config.gameScriptPath.relativePath, LOG_ERROR);
            log("Resolved from config dir: " + g_config.configDir, LOG_ERROR);
            allValid = false;
        }
        else {
            log("Game script path validated", LOG_OK);
            if (!isSilentMode()) {
                log("  Relative: " + g_config.gameScriptPath.relativePath);
                log("  Absolute: " + g_config.gameScriptPath.absolutePath);
            }
        }
    }
    catch (const std::exception& e) {
        log("Cannot access gameScriptPath: " + std::string(e.what()), LOG_ERROR);
        log("Relative path: " + g_config.gameScriptPath.relativePath, LOG_ERROR);
        log("Absolute path: " + g_config.gameScriptPath.absolutePath, LOG_ERROR);
        allValid = false;
    }

    // Validate modulePath
    try {
        if (!fs::exists(g_config.modulePath.absolutePath)) {
            log("Creating modulePath directory: " + g_config.modulePath.absolutePath);
            fs::create_directories(g_config.modulePath.absolutePath);
        }

        if (!fs::is_directory(g_config.modulePath.absolutePath)) {
            log("modulePath is not a directory, using gameScriptPath");
            g_config.modulePath = g_config.gameScriptPath;
        }
        else {
            log("Module path validated", LOG_OK);
            if (!isSilentMode()) {
                log("  Relative: " + g_config.modulePath.relativePath);
                log("  Absolute: " + g_config.modulePath.absolutePath);
            }
        }
    }
    catch (const std::exception& e) {
        log("Cannot access modulePath, using gameScriptPath: " + std::string(e.what()));
        g_config.modulePath = g_config.gameScriptPath;
    }

    return allValid;
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:

        InitConsole();

        g_hModule = hMod;

        logBranding();

        if (!initializePaths()) {
            log("Initialization failed - check your .me3 config file", LOG_ERROR);
            log("Example .me3 config:", LOG_ERROR);
            log("  gameScriptPath = \"relative/path/to/script\"", LOG_ERROR);
            log("  modulePath = \"relative/path/to/modules\"", LOG_ERROR);
            log("  silent = false", LOG_ERROR);
            break;
        }

        if (!validatePaths()) {
            log("Path validation had issues, but continuing...", LOG_ERROR);
        }

        // Always clear the flag file on DLL load to ensure fresh module loading
        clearModuleLoadedFlag(g_config.modulePath.absolutePath);

        createWorkingSetupScript(g_config);
        injectIntoHksFile(g_config);

        // Register cleanup function for process exit
        atexit(cleanup);

        if (!isSilentMode()) {
            log("Ready! Modules will load automatically.");
            log("Using relative paths from: " + fs::path(g_config.configFile).filename().string());
            log("Relative paths resolved from config directory: " + g_config.configDir);
            log("Flag file will be cleared on each game restart.");
            log("");
        }
        break;

    case DLL_PROCESS_DETACH:
        // Clean up flag file on process detach
        cleanup();
        break;
    }
    return TRUE;
}