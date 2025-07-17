// =============================================
// File: HksInjector.cpp
// Category: HKS Script Integration
// Purpose: Implements injection of Lua loader into the target .hks file.
// =============================================
#include "HksInjector.h"
#include "PathUtils.h"
#include "Logger.h"
#include "ConfigParser.h"  // For validateHKSForBackup function
#include "ErrorMessages.h"  // For clean error formatting
#include <filesystem>
#include <fstream>
#include <ctime>
#include <sstream>

namespace fs = std::filesystem;

// Universal HKS backup function with context support and validation
bool createHksBackup(const std::string& hksPath, const LoaderConfig& config, const std::string& context) {
    // Validate HKS file before attempting backup
    if (!validateHKSForBackup(hksPath)) {
        log("HKS file validation failed, skipping backup: " + hksPath, LOG_WARNING, "HksInjector");
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

// Enhanced injection detection - returns detailed info about what was found
struct InjectionStatus {
    bool isInjected;
    std::string matchedPattern;
    std::string matchType;
};

InjectionStatus checkInjectionStatus(const std::string& fileContent, const std::string& injectionLine) {
    // Check for exact injection line (current version)
    if (fileContent.find(injectionLine) != std::string::npos) {
        return { true, injectionLine, "exact current injection" };
    }

    // Check for any reference to module_loader_setup.lua (any version/format)
    if (fileContent.find("module_loader_setup.lua") != std::string::npos) {
        return { true, "module_loader_setup.lua", "module loader reference" };
    }

    // Check for other common injection patterns from previous versions
    std::vector<std::pair<std::string, std::string>> legacyPatterns = {
        {"-- Lua Loader by Malice", "legacy header signature"},
        {"dofile('", "legacy dofile single quotes"},
        {"dofile(\"", "legacy dofile double quotes"},
        {"_module_loader", "legacy module loader reference"}
    };

    for (const auto& [pattern, description] : legacyPatterns) {
        if (fileContent.find(pattern) != std::string::npos) {
            return { true, pattern, description };
        }
    }

    return { false, "", "" };
}

void injectIntoHksFile(const LoaderConfig& config) {
    // FIXED: Handle empty gameScriptPath with proper error message instead of silent return
    if (config.gameScriptPath.absolutePath.empty()) {
        log(ErrorMessages::formatEmptyGameScriptPathError(config.configFile), LOG_BRAND);
        return;
    }

    std::string hksPath = config.gameScriptPath.absolutePath + "/c0000.hks";

    // Check if HKS file exists and is accessible
    try {
        if (!fs::exists(hksPath) || !fs::is_regular_file(hksPath)) {
            log(ErrorMessages::formatHksNotFoundError(hksPath, config), LOG_BRAND);
            return;
        }
    }
    catch (const std::exception& e) {
        log(ErrorMessages::formatHksAccessError(hksPath, e.what()), LOG_BRAND);
        return;
    }
    catch (...) {
        log(ErrorMessages::formatHksSystemError(hksPath), LOG_BRAND);
        return;
    }

    // FIXED: Enhanced file reading with proper error handling
    std::string fileContent;
    try {
        std::ifstream hksRead(hksPath, std::ios::binary);
        if (!hksRead.is_open()) {
            log(ErrorMessages::formatHksReadError(hksPath), LOG_BRAND);
            return;
        }

        // Read file content with proper error checking
        hksRead.seekg(0, std::ios::end);
        size_t fileSize = hksRead.tellg();
        hksRead.seekg(0, std::ios::beg);

        fileContent.reserve(fileSize);
        fileContent.assign((std::istreambuf_iterator<char>(hksRead)), std::istreambuf_iterator<char>());

        if (hksRead.bad()) {
            log(ErrorMessages::formatHksReadError(hksPath), LOG_BRAND);
            return;
        }

        hksRead.close();
        log("Successfully read HKS file (" + std::to_string(fileSize) + " bytes)", LOG_DEBUG, "HksInjector");
    }
    catch (const std::exception& e) {
        log(ErrorMessages::formatHksAccessError(hksPath, "Read error: " + std::string(e.what())), LOG_BRAND);
        return;
    }
    catch (...) {
        log(ErrorMessages::formatHksSystemError(hksPath), LOG_BRAND);
        return;
    }

    // Create injection line using absolute path (required for dofile)
    std::string setupScriptPath = config.modulePath.absolutePath + "/_module_loader/module_loader_setup.lua";
    std::string injectionLine = "dofile('" + setupScriptPath + "')";

    // IMPROVED: Enhanced injection detection with detailed diagnostics
    // WHY USE OR LOGIC: This checks for multiple injection patterns to prevent duplicates:
    // 1. Exact current injection line (prevents duplicate injection)
    // 2. Any reference to module_loader_setup.lua (detects manual or previous installations)
    // 3. Legacy patterns from previous versions (ensures compatibility)
    // This is GOOD PRACTICE because it's defensive programming - better to skip than duplicate
    InjectionStatus injectionStatus = checkInjectionStatus(fileContent, injectionLine);
    if (injectionStatus.isInjected) {
        log("Already integrated with game script", LOG_INFO, "HksInjector");
        log("Found: " + injectionStatus.matchedPattern + " (" + injectionStatus.matchType + ")", LOG_DEBUG, "HksInjector");

        // Only backup if backupHKSonLaunch is true (always backup mode)
        // Uses validation to prevent empty backups
        if (config.backupHKSonLaunch) {
            if (createHksBackup(hksPath, config, "launch")) {
                log("Launch backup completed successfully", LOG_DEBUG, "HksInjector");
            }
            else {
                log("Launch backup skipped or failed", LOG_DEBUG, "HksInjector");
            }
        }

        log("Injection operation completed - no changes needed for " + fs::path(hksPath).filename().string(), LOG_INFO, "HksInjector");
        return;
    }

    // We're going to inject - backup regardless of setting since we're modifying the file
    // But still validate to ensure we don't backup empty files
    if (createHksBackup(hksPath, config, "injection")) {
        log("Pre-injection backup completed successfully", LOG_DEBUG, "HksInjector");
    }
    else {
        log("Pre-injection backup skipped or failed - proceeding with injection", LOG_WARNING, "HksInjector");
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

    // FIXED: Enhanced file writing with comprehensive exception handling
    try {
        std::ofstream hksWrite(hksPath, std::ios::binary);
        if (!hksWrite.is_open()) {
            log(ErrorMessages::formatHksWriteError(hksPath, "Unable to open file for writing"), LOG_BRAND);
            return;
        }

        // Write content with proper error checking
        hksWrite << newContent;

        // Explicitly check for write errors before flushing
        if (hksWrite.bad()) {
            hksWrite.close();
            log(ErrorMessages::formatHksWriteError(hksPath, "Write operation failed"), LOG_BRAND);
            return;
        }

        // Flush with error checking (this is sufficient for catching write errors)
        hksWrite.flush();
        if (hksWrite.bad()) {
            hksWrite.close();
            log(ErrorMessages::formatHksWriteError(hksPath, "Flush operation failed"), LOG_BRAND);
            return;
        }

        hksWrite.close();
        // Note: .bad() check after .close() removed as .flush() is sufficient

        log("Successfully integrated with game script", LOG_INFO, "HksInjector");
        log("Injection uses absolute path: " + setupScriptPath, LOG_DEBUG, "HksInjector");
        log("Config uses relative paths for portability", LOG_DEBUG, "HksInjector");
        log("Injection operation completed successfully for " + fs::path(hksPath).filename().string(), LOG_INFO, "HksInjector");
    }
    catch (const std::ios_base::failure& e) {
        log(ErrorMessages::formatHksWriteError(hksPath, "I/O error: " + std::string(e.what())), LOG_BRAND);
        return;
    }
    catch (const std::exception& e) {
        log(ErrorMessages::formatHksWriteError(hksPath, "Write error: " + std::string(e.what())), LOG_BRAND);
        return;
    }
    catch (...) {
        log(ErrorMessages::formatHksWriteSystemError(hksPath), LOG_BRAND);
        return;
    }
}