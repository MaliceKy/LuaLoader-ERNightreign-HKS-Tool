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
#include <sstream>

namespace fs = std::filesystem;

// Universal HKS backup function with context support
bool createHksBackup(const std::string& hksPath, const LoaderConfig& config, const std::string& context) {
    if (!fs::exists(hksPath)) {
        log("Backup target does not exist: " + hksPath, LOG_DEBUG, "HksInjector");
        return false;
    }

    // Generate backup filename with consistent date format
    auto now = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &now);

    char dateStr[32];
    std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d_%H-%M-%S", &tm);

    std::ostringstream backupName;
    backupName << fs::path(hksPath).filename().string() << ".backup_" << dateStr;

    // Add context for why backup was created
    if (!context.empty()) {
        backupName << "_" << context;
    }

    // Determine backup directory using same logic as existing system
    std::string backupDir;
    if (config.backupHKSFolder.empty()) {
        // Use same directory as the original file
        backupDir = fs::path(hksPath).parent_path().string();
    }
    else {
        // Use configured backup folder (resolve relative to config directory)
        backupDir = resolvePathWithFallbacks(config.backupHKSFolder, config.configDir);
    }

    // Ensure backup directory exists
    try {
        fs::create_directories(backupDir);
    }
    catch (const std::exception& e) {
        log("Failed to create backup directory: " + std::string(e.what()), LOG_ERROR, "HksInjector");
        return false;
    }

    std::string backupPath = normalizePath(backupDir + "/" + backupName.str());

    try {
        fs::copy_file(hksPath, backupPath, fs::copy_options::overwrite_existing);
        log("Backup created: " + backupPath, LOG_INFO, "HksInjector");
        return true;
    }
    catch (const std::exception& e) {
        log("Backup creation failed: " + std::string(e.what()), LOG_ERROR, "HksInjector");
        return false;
    }
}

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

    // Create injection line using absolute path (required for dofile)
    std::string setupScriptPath = config.modulePath.absolutePath + "/_module_loader/module_loader_setup.lua";
    std::string injectionLine = "dofile('" + setupScriptPath + "')";

    // Check if already injected
    bool alreadyInjected = (fileContent.find(injectionLine) != std::string::npos ||
        fileContent.find("module_loader_setup.lua") != std::string::npos);

    if (alreadyInjected) {
        log("Already integrated with game script", LOG_INFO, "HksInjector");

        // Only backup if backupHKSonLaunch is true (always backup mode)
        if (config.backupHKSonLaunch) {
            createHksBackup(hksPath, config, "launch");
        }
        return;
    }

    // We're going to inject - backup regardless of setting since we're modifying the file
    createHksBackup(hksPath, config, "injection");

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