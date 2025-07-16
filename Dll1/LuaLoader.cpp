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
        log("Failed to get DLL path", LOG_ERROR, "LuaLoader");
        return false;
    }
    fs::path dllPath(buf);

    log("DLL location: " + normalizePath(dllPath.parent_path().string()), LOG_DEBUG, "LuaLoader");
    log("Searching for .me3 config files...", LOG_DEBUG, "LuaLoader");

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
        log("Searching: " + normalizePath(dir.string()), LOG_TRACE, "LuaLoader");

        try {
            if (!fs::exists(dir) || !fs::is_directory(dir)) {
                continue;
            }

            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".me3") {
                    me3Path = entry.path();
                    log("Found .me3 file: " + me3Path.filename().string(), LOG_DEBUG, "LuaLoader");
                    break;
                }
            }
        }
        catch (const std::exception& e) {
            log("Cannot access directory " + dir.string() + ": " + e.what(), LOG_TRACE, "LuaLoader");
            continue;
        }
        if (!me3Path.empty()) break;
    }

    if (me3Path.empty()) {
        log("No .me3 configuration file found!", LOG_ERROR, "LuaLoader");
        log("Create a .me3 file with gameScriptPath and modulePath", LOG_ERROR, "LuaLoader");
        log("Search paths checked:", LOG_ERROR, "LuaLoader");
        for (const auto& path : searchPaths) {
            log("  " + path.string(), LOG_ERROR, "LuaLoader");
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
        log("Using custom config path from .me3: " + configPath.string(), LOG_DEBUG, "LuaLoader");
    }
    else {
        // 3. Use default path next to .me3 file
        configPath = configDir / "LuaLoader.toml";
        log("Using default config path: " + configPath.string(), LOG_DEBUG, "LuaLoader");
    }

    // 4. If config does not exist, generate it
    if (!fs::exists(configPath)) {
        log("Configuration file not found, generating default config", LOG_INFO, "LuaLoader");
        log("Config will be created at: " + configPath.string(), LOG_INFO, "LuaLoader");

        // Ensure parent directory exists
        try {
            fs::create_directories(configPath.parent_path());
        }
        catch (const std::exception& e) {
            log("Failed to create config directory: " + std::string(e.what()), LOG_ERROR, "LuaLoader");
            return false;
        }

        generateDefaultConfigToml(configPath.string());
        injectTomlPathToMe3(me3Path.string(), configPath.string());

        log("Default config generated successfully!", LOG_INFO, "LuaLoader");
        log("Please edit the configuration file and restart to complete setup", LOG_INFO, "LuaLoader");
        return false; // Ask user to edit config
    }

    // 5. Parse the config
    if (!parseTomlConfig(configPath.string(), g_config)) {
        log("Config parsing failed. Please check " + configPath.string(), LOG_ERROR, "LuaLoader");
        return false;
    }

    log("Configuration loaded: " + configPath.filename().string(), LOG_INFO, "LuaLoader");
    log("Config directory: " + configDir.string(), LOG_DEBUG, "LuaLoader");
    return true;
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        log("DLL_PROCESS_ATTACH - Starting initialization", LOG_TRACE, "LuaLoader");

        InitConsole();

        g_hModule = hMod;

        logBranding();

        if (!initializePaths()) {
            log("Initialization failed - check your configuration", LOG_ERROR, "LuaLoader");
            log("", LOG_ERROR, "LuaLoader");
            log("Configuration process:", LOG_ERROR, "LuaLoader");
            log("1. Create a .me3 file with basic config", LOG_ERROR, "LuaLoader");
            log("2. LuaLoader.toml will be auto-generated", LOG_ERROR, "LuaLoader");
            log("3. Edit LuaLoader.toml and relaunch", LOG_ERROR, "LuaLoader");
            log("", LOG_ERROR, "LuaLoader");
            log("For custom config location, add to .me3:", LOG_ERROR, "LuaLoader");
            log("  luaLoaderConfigPath = \"path/to/config.toml\"", LOG_ERROR, "LuaLoader");
            break;
        }

        if (!validatePaths(g_config)) {
            log("Path validation had issues, but continuing...", LOG_WARNING, "LuaLoader");
        }

        // Always clear the flag file on DLL load to ensure fresh module loading
        clearModuleLoadedFlag(g_config.modulePath.absolutePath);
        log("Cleared module loaded flag for fresh reload", LOG_DEBUG, "LuaLoader");

        log("Creating setup script...", LOG_DEBUG, "LuaLoader");
        createWorkingSetupScript(g_config);

        log("Injecting into HKS file...", LOG_DEBUG, "LuaLoader");
        injectIntoHksFile(g_config);

        // Register cleanup function for process exit
        atexit(cleanup);

        log("Initialization complete - ready for module loading", LOG_INFO, "LuaLoader");
        log("Config: " + fs::path(g_config.configFile).filename().string() + " | Modules will load when game script runs", LOG_INFO, "LuaLoader");
        log("==========================================", LOG_INFO, "LuaLoader");
        break;

    case DLL_PROCESS_DETACH:
        log("DLL_PROCESS_DETACH - Cleaning up", LOG_TRACE, "LuaLoader");
        // Clean up flag file on process detach
        cleanup();
        break;
    }
    return TRUE;
}