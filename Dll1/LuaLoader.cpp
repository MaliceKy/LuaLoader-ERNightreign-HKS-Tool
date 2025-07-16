// =============================================
// File: LuaLoader.cpp
// Category: Main Loader Orchestration
// Purpose: Orchestrates DLL lifecycle and module loading logic (entry point).
// =============================================
#include "Console.h"
#include "ConfigGenerator.h"
#include "Me3Utils.h"
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

    if (!isSilentMode()) {
        log("DLL location: " + normalizePath(dllPath.parent_path().string()));
        log("Searching for .me3 config files...");
    }

    // 1. Search for the first .me3 file in standard locations
    std::vector<fs::path> searchPaths = {
        dllPath.parent_path(),
        dllPath.parent_path().parent_path(),
        dllPath.parent_path().parent_path().parent_path(),
        fs::current_path(),
        fs::path("C:/"),
    };

    fs::path me3Path;
    for (const auto& dir : searchPaths) {
        if (!isSilentMode()) {
            log("Searching: " + normalizePath(dir.string()));
        }

        try {
            if (!fs::exists(dir) || !fs::is_directory(dir)) {
                continue;
            }

            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".me3") {
                    me3Path = entry.path();
                    if (!isSilentMode()) {
                        log("Found .me3 file: " + me3Path.filename().string());
                    }
                    break;
                }
            }
        }
        catch (const std::exception& e) {
            if (!isSilentMode()) {
                log("Cannot access directory " + dir.string() + ": " + e.what());
            }
            continue;
        }
        if (!me3Path.empty()) break;
    }

    if (me3Path.empty()) {
        log("No .me3 configuration file found!", LOG_ERROR);
        log("Create a .me3 file with gameScriptPath and modulePath", LOG_ERROR);
        log("Search paths checked:", LOG_ERROR);
        for (const auto& path : searchPaths) {
            log("  " + path.string(), LOG_ERROR);
        }
        return false;
    }

    fs::path configDir = me3Path.parent_path();
    fs::path configPath;

    // 2. Check for path override in .me3 file
    std::string overridePath = parseConfigPathFromMe3(me3Path);
    if (!overridePath.empty()) {
        configPath = overridePath;
        // If relative path, resolve it relative to the .me3 file
        if (configPath.is_relative()) {
            configPath = configDir / configPath;
        }
        if (!isSilentMode()) {
            log("Using custom config path from .me3: " + configPath.string());
        }
    }
    else {
        // 3. Use default path next to .me3 file
        configPath = configDir / "LuaLoader.toml";
        if (!isSilentMode()) {
            log("Using default config path: " + configPath.string());
        }
    }

    // 4. If config does not exist, generate it
    if (!fs::exists(configPath)) {
        log("No LuaLoader.toml config found. Generating default at: " + configPath.string(), LOG_ERROR);

        // Ensure parent directory exists
        try {
            fs::create_directories(configPath.parent_path());
        }
        catch (const std::exception& e) {
            log("Failed to create config directory: " + std::string(e.what()), LOG_ERROR);
            return false;
        }

        generateDefaultConfigToml(configPath.string());
        injectTomlPathToMe3(me3Path.string(), configPath.string());

        log("Default config generated! Edit and relaunch to set up.", LOG_OK);
        return false; // Ask user to edit config
    }

    // 5. Parse the config
    if (!parseTomlConfig(configPath.string(), g_config)) {
        log("Config parsing failed. Please check " + configPath.string(), LOG_ERROR);
        return false;
    }

    log("Config loaded: " + configPath.filename().string(), LOG_OK);
    if (!isSilentMode()) {
        log("Config directory: " + configDir.string());
    }
    return true;
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:

        InitConsole();

        g_hModule = hMod;

        logBranding();

        if (!initializePaths()) {
            log("Initialization failed - check your configuration", LOG_ERROR);
            log("", LOG_ERROR);
            log("Configuration process:", LOG_ERROR);
            log("1. Create a .me3 file with basic config", LOG_ERROR);
            log("2. LuaLoader.toml will be auto-generated", LOG_ERROR);
            log("3. Edit LuaLoader.toml and relaunch", LOG_ERROR);
            log("", LOG_ERROR);
            log("For custom config location, add to .me3:", LOG_ERROR);
            log("  luaLoaderConfigPath = \"path/to/config.toml\"", LOG_ERROR);
            break;
        }

        if (!validatePaths(g_config)) {
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
            log("Using config file: " + fs::path(g_config.configFile).filename().string());
            log("Config directory: " + g_config.configDir);
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