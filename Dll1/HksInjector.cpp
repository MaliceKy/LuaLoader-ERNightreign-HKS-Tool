// =============================================
// File: HksInjector.cpp
// Category: HKS Script Integration
// Purpose: Implements injection of Lua loader into the target .hks file.
// =============================================
#include "HksInjector.h"
#include "PathUtils.h"
#include "Logger.h"
#include <filesystem>
#include <fstream>
#include <ctime>

namespace fs = std::filesystem;

void injectIntoHksFile(const LoaderConfig& config) {
    if (config.gameScriptPath.absolutePath.empty()) return;

    std::string hksPath = config.gameScriptPath.absolutePath + "/c0000.hks";

    try {
        if (!fs::exists(hksPath) || !fs::is_regular_file(hksPath)) {
            log("c0000.hks not found, creating empty file: " + hksPath);

            // Create the directory if it doesn't exist
            fs::create_directories(fs::path(hksPath).parent_path());

            // Create empty c0000.hks
            std::ofstream newHks(hksPath);
            if (newHks.is_open()) {
                newHks << "-- Auto-generated c0000.hks\n";
                newHks.close();
            }
            else {
                log("Cannot create c0000.hks", LOG_ERROR);
                return;
            }
        }
    }
    catch (...) {
        log("Cannot access c0000.hks", LOG_ERROR);
        return;
    }

    // Read current file
    std::ifstream hksRead(hksPath, std::ios::binary);
    if (!hksRead.is_open()) {
        log("Cannot read c0000.hks", LOG_ERROR);
        return;
    }

    std::string fileContent((std::istreambuf_iterator<char>(hksRead)), std::istreambuf_iterator<char>());
    hksRead.close();

    // Create backup with timestamp (ALWAYS runs if enabled, before checking injection)
    if (config.backupHKSonLaunch) {
        // Determine backup folder
        std::string backupDir;
        if (!config.backupHKSFolder.empty()) {
            // If relative, resolve relative to config directory
            backupDir = resolvePathWithFallbacks(config.backupHKSFolder, config.configDir);
            try {
                fs::create_directories(backupDir);
            }
            catch (...) { /* ignore */ }
        }
        else {
            // Default: same folder as hksPath
            backupDir = fs::path(hksPath).parent_path().string();
        }
        std::string backupFilename = fs::path(hksPath).filename().string() + ".backup_" + std::to_string(time(nullptr));
        std::string backupPath = normalizePath(backupDir + "/" + backupFilename);
        std::ofstream backup(backupPath, std::ios::binary);
        if (backup.is_open()) {
            backup << fileContent;
            backup.close();
            log("Backup created at: " + backupPath, LOG_OK);
        }
        else {
            log("Failed to create backup at: " + backupPath, LOG_ERROR);
        }
    }
    else {
        log("Backup on launch is disabled by config.");
    }

    // Create injection line using absolute path (required for dofile)
    std::string setupScriptPath = config.modulePath.absolutePath + "/_module_loader/module_loader_setup.lua";
    std::string injectionLine = "dofile('" + setupScriptPath + "')";

    // Check if already injected
    if (fileContent.find(injectionLine) != std::string::npos ||
        fileContent.find("module_loader_setup.lua") != std::string::npos) {
        log("Already integrated with game script", LOG_OK);
        return;
    }

    // Create informative header with enhanced path information
    std::string header = "-- Auto-injected by Lua Loader v11.3 (Enhanced Path Resolution)\n";
    header += "-- Config file: " + fs::path(config.configFile).filename().string() + "\n";
    header += "-- Config dir: " + config.configDir + "\n";
    header += "-- Game script path (relative): " + config.gameScriptPath.relativePath + "\n";
    header += "-- Game script path (absolute): " + config.gameScriptPath.absolutePath + "\n";
    header += "-- Module path (relative): " + config.modulePath.relativePath + "\n";
    header += "-- Module path (absolute): " + config.modulePath.absolutePath + "\n\n";

    std::string newContent = header + injectionLine + "\n\n" + fileContent;

    // Write modified file
    std::ofstream hksWrite(hksPath, std::ios::binary);
    if (!hksWrite.is_open()) {
        log("Cannot write to c0000.hks", LOG_ERROR);
        return;
    }

    hksWrite << newContent;
    hksWrite.close();

    log("Successfully integrated with game script", LOG_OK);
    if (!isSilentMode()) {
        log("Injection uses absolute path: " + setupScriptPath);
        log("Config uses relative paths for portability");
    }
}