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
            log("c0000.hks not found, creating empty file: " + hksPath, LOG_INFO, "HksInjector");

            // Create the directory if it doesn't exist
            fs::create_directories(fs::path(hksPath).parent_path());

            // Create empty c0000.hks
            std::ofstream newHks(hksPath);
            if (newHks.is_open()) {
                newHks << "-- Auto-generated c0000.hks\n";
                newHks.close();
            }
            else {
                log("Cannot create c0000.hks", LOG_ERROR, "HksInjector");
                return;
            }
        }
    }
    catch (const std::exception& e) {
        log("Cannot access c0000.hks: " + std::string(e.what()), LOG_ERROR, "HksInjector");
        return;
    }
    catch (...) {
        log("Cannot access c0000.hks: unknown error", LOG_ERROR, "HksInjector");
        return;
    }

    // Read current file
    std::ifstream hksRead(hksPath, std::ios::binary);
    if (!hksRead.is_open()) {
        log("Cannot read c0000.hks", LOG_ERROR, "HksInjector");
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
            catch (const std::exception& e) {
                log("Failed to create backup directory: " + std::string(e.what()), LOG_WARNING, "HksInjector");
            }
            catch (...) {
                log("Failed to create backup directory: unknown error", LOG_WARNING, "HksInjector");
            }
        }
        else {
            // Default: same folder as hksPath
            backupDir = fs::path(hksPath).parent_path().string();
        }

        // Create human-readable timestamp
        std::time_t now = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &now);
        char timeStr[32];
        std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d_%H-%M-%S", &tm);

        std::string backupFilename = fs::path(hksPath).filename().string() + ".backup_" + std::string(timeStr);
        std::string backupPath = normalizePath(backupDir + "/" + backupFilename);

        std::ofstream backup(backupPath, std::ios::binary);
        if (backup.is_open()) {
            backup << fileContent;
            backup.close();
            log("Backup created at: " + backupPath, LOG_INFO, "HksInjector");
        }
        else {
            log("Failed to create backup at: " + backupPath, LOG_ERROR, "HksInjector");
        }
    }
    else {
        log("Backup on launch is disabled by config", LOG_DEBUG, "HksInjector");
    }

    // Create injection line using absolute path (required for dofile)
    std::string setupScriptPath = config.modulePath.absolutePath + "/_module_loader/module_loader_setup.lua";
    std::string injectionLine = "dofile('" + setupScriptPath + "')";

    // Check if already injected
    if (fileContent.find(injectionLine) != std::string::npos ||
        fileContent.find("module_loader_setup.lua") != std::string::npos) {
        log("Already integrated with game script", LOG_INFO, "HksInjector");
        return;
    }

    // Create clean, professional header
    std::string header = "-- ========================================\n";
    header += "-- Lua Loader v11.3 - Enhanced Path Resolution\n";
    header += "-- by Malice\n";
    header += "-- ========================================\n";
    header += "-- Config: " + fs::path(config.configFile).filename().string() + "\n";
    header += "-- Module Path: " + config.modulePath.relativePath + "\n";
    header += "-- ========================================\n\n";

    std::string newContent = header + injectionLine + "\n\n" + fileContent;

    // Write modified file
    std::ofstream hksWrite(hksPath, std::ios::binary);
    if (!hksWrite.is_open()) {
        log("Cannot write to c0000.hks", LOG_ERROR, "HksInjector");
        return;
    }

    hksWrite << newContent;
    hksWrite.close();

    log("Successfully integrated with game script", LOG_INFO, "HksInjector");
    log("Injection uses absolute path: " + setupScriptPath, LOG_DEBUG, "HksInjector");
    log("Config uses relative paths for portability", LOG_DEBUG, "HksInjector");
}