// =============================================
// File: Cleanup.cpp
// Category: Cleanup Utilities
// Purpose: Implements cleanup functions for removing loader artifacts
// =============================================
#include "Cleanup.h"
#include "HksInjector.h"  // For universal backup function
#include "Logger.h"
#include "PathUtils.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace {
    // Utility: Read file content into vector of lines
    std::vector<std::string> readFileLines(const std::string& filePath) {
        std::vector<std::string> lines;
        std::ifstream file(filePath);

        if (!file.is_open()) {
            log("Failed to open file for reading: " + filePath, LOG_ERROR, "Cleanup");
            return lines;
        }

        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        return lines;
    }

    // Utility: Write vector of lines to file
    bool writeFileLines(const std::string& filePath, const std::vector<std::string>& lines) {
        std::ofstream file(filePath);

        if (!file.is_open()) {
            log("Failed to open file for writing: " + filePath, LOG_ERROR, "Cleanup");
            return false;
        }

        for (const auto& line : lines) {
            file << line << "\n";
        }
        file.close();

        return true;
    }
}

namespace Cleanup {

    bool performFullCleanup(LoaderConfig& config) {
        log("==========================================", LOG_INFO, "Cleanup");
        log("LuaLoader cleanup operation initiated", LOG_INFO, "Cleanup");
        log("Removing loader-generated artifacts only", LOG_INFO, "Cleanup");
        log("==========================================", LOG_INFO, "Cleanup");

        bool allOperationsSuccessful = true;
        int operationsCompleted = 0;
        int totalOperations = 3;

        // Operation 1: Remove _module_loader directory
        if (!config.modulePath.absolutePath.empty()) {
            log("Starting module loader directory cleanup", LOG_INFO, "Cleanup");
            if (cleanupModuleLoaderDirectory(config.modulePath.absolutePath)) {
                operationsCompleted++;
            }
            else {
                log("Module loader directory cleanup encountered issues", LOG_WARNING, "Cleanup");
                allOperationsSuccessful = false;
            }
        }
        else {
            log("Module path not configured - skipping directory cleanup", LOG_DEBUG, "Cleanup");
            operationsCompleted++;
        }

        // Operation 2: Remove flag files
        if (!config.modulePath.absolutePath.empty()) {
            log("Starting flag file cleanup", LOG_INFO, "Cleanup");
            if (cleanupFlagFiles(config.modulePath.absolutePath)) {
                operationsCompleted++;
            }
            else {
                log("Flag file cleanup encountered issues", LOG_WARNING, "Cleanup");
                allOperationsSuccessful = false;
            }
        }
        else {
            log("Module path not configured - skipping flag file cleanup", LOG_DEBUG, "Cleanup");
            operationsCompleted++;
        }

        // Operation 3: Clean HKS injection
        if (!config.gameScriptPath.absolutePath.empty()) {
            std::string hksPath = config.gameScriptPath.absolutePath + "/c0000.hks";
            log("Starting HKS injection cleanup", LOG_INFO, "Cleanup");

            if (fs::exists(hksPath)) {
                // Use universal backup function with cleanup context
                createHksBackup(hksPath, config, "cleanup");

                if (cleanupHksInjection(hksPath)) {
                    operationsCompleted++;
                }
                else {
                    log("HKS injection cleanup encountered issues", LOG_WARNING, "Cleanup");
                    allOperationsSuccessful = false;
                }
            }
            else {
                log("HKS file not found - no injection to clean", LOG_DEBUG, "Cleanup");
                operationsCompleted++;
            }
        }
        else {
            log("Game script path not configured - skipping HKS cleanup", LOG_DEBUG, "Cleanup");
            operationsCompleted++;
        }

        // Final status report
        log("==========================================", LOG_INFO, "Cleanup");

        if (allOperationsSuccessful && operationsCompleted == totalOperations) {
            log("Cleanup completed successfully", LOG_INFO, "Cleanup");
            log("All loader artifacts have been removed", LOG_INFO, "Cleanup");
        }
        else if (operationsCompleted > 0) {
            log("Cleanup completed with warnings", LOG_WARNING, "Cleanup");
            log("Some artifacts may require manual removal", LOG_WARNING, "Cleanup");
        }
        else {
            log("Cleanup failed - no operations completed", LOG_ERROR, "Cleanup");
        }

        log("==========================================", LOG_INFO, "Cleanup");

        return allOperationsSuccessful;
    }

    bool cleanupModuleLoaderDirectory(const std::string& modulePath) {
        std::string loaderDirectory = modulePath + "/_module_loader";

        try {
            if (!fs::exists(loaderDirectory)) {
                log("Module loader directory not found (already clean)", LOG_DEBUG, "Cleanup");
                return true;
            }

            // Count items for logging
            size_t itemCount = 0;
            try {
                for (const auto& entry : fs::recursive_directory_iterator(loaderDirectory)) {
                    if (entry.is_regular_file()) itemCount++;
                }
            }
            catch (const std::exception& e) {
                log("Warning: Could not count files in directory: " + std::string(e.what()), LOG_WARNING, "Cleanup");
            }

            fs::remove_all(loaderDirectory);

            if (itemCount > 0) {
                log("Removed _module_loader directory with " + std::to_string(itemCount) + " files", LOG_INFO, "Cleanup");
            }
            else {
                log("Removed _module_loader directory", LOG_INFO, "Cleanup");
            }

            return true;

        }
        catch (const std::exception& e) {
            log("Failed to remove _module_loader directory: " + std::string(e.what()), LOG_ERROR, "Cleanup");
            return false;
        }
    }

    bool cleanupFlagFiles(const std::string& modulePath) {
        std::vector<std::string> flagFilePaths = {
            modulePath + "/_module_loader/.modules_loaded",
            modulePath + "/.modules_loaded"
        };

        bool allFilesProcessed = true;
        int filesRemoved = 0;

        for (const auto& flagPath : flagFilePaths) {
            try {
                if (fs::exists(flagPath)) {
                    fs::remove(flagPath);
                    filesRemoved++;
                    log("Removed flag file: " + fs::path(flagPath).filename().string(), LOG_INFO, "Cleanup");
                }
            }
            catch (const std::exception& e) {
                log("Failed to remove flag file " + flagPath + ": " + std::string(e.what()), LOG_ERROR, "Cleanup");
                allFilesProcessed = false;
            }
        }

        if (filesRemoved == 0) {
            log("No flag files found (already clean)", LOG_DEBUG, "Cleanup");
        }

        return allFilesProcessed;
    }

    bool cleanupHksInjection(const std::string& hksPath) {
        if (!fs::exists(hksPath)) {
            log("HKS file not found: " + hksPath, LOG_DEBUG, "Cleanup");
            return true;
        }

        auto lines = readFileLines(hksPath);
        if (lines.empty()) {
            log("Unable to read HKS file content", LOG_ERROR, "Cleanup");
            return false;
        }

        log("Read " + std::to_string(lines.size()) + " lines from HKS file", LOG_DEBUG, "Cleanup");

        std::vector<std::string> cleanedLines;
        bool insideInjectionBlock = false;
        bool injectionFound = false;
        int startLine = -1, endLine = -1;
        int i = 0;
        for (; i < lines.size(); ++i) {
            const std::string& line = lines[i];
            // Start block
            if (!insideInjectionBlock &&
                line.find("-- ========================================") != std::string::npos &&
                i + 1 < lines.size() &&
                lines[i + 1].find("-- Lua Loader") != std::string::npos) {
                insideInjectionBlock = true;
                injectionFound = true;
                startLine = i + 1;
                log("Found injection start at line " + std::to_string(startLine), LOG_DEBUG, "Cleanup");
                continue; // skip start marker line
            }
            if (insideInjectionBlock) {
                // Look for the dofile line (end of block)
                if (line.find("dofile(") != std::string::npos &&
                    line.find("module_loader_setup.lua") != std::string::npos) {
                    endLine = i + 1;
                    insideInjectionBlock = false;
                    log("Found injection end at line " + std::to_string(endLine), LOG_DEBUG, "Cleanup");
                    // Skip dofile line too
                    // Now, ALSO skip any consecutive blank lines after this block
                    int skipBlanks = i + 1;
                    while (skipBlanks < lines.size() && lines[skipBlanks].find_first_not_of(" \t\r\n") == std::string::npos) {
                        ++skipBlanks;
                    }
                    i = skipBlanks - 1; // will increment to next real line at loop top
                    continue;
                }
                // Skip everything inside injection block
                continue;
            }
            // Not inside block, keep line
            cleanedLines.push_back(line);
        }

        if (!injectionFound) {
            log("No LuaLoader injection found in HKS file", LOG_DEBUG, "Cleanup");
            return true;
        }

        if (insideInjectionBlock) {
            log("Warning: Injection block was not properly closed (missing dofile line)", LOG_WARNING, "Cleanup");
        }

        if (writeFileLines(hksPath, cleanedLines)) {
            log("Removed LuaLoader injection (and trailing blank lines)", LOG_INFO, "Cleanup");
            if (startLine > 0)
                log("Injection was between lines " + std::to_string(startLine) +
                    " and " + std::to_string(endLine > 0 ? endLine : startLine), LOG_DEBUG, "Cleanup");
            return true;
        }
        else {
            log("Failed to write cleaned HKS file", LOG_ERROR, "Cleanup");
            return false;
        }
    }

    void debugHksFile(const std::string& hksPath) {
        if (!fs::exists(hksPath)) {
            log("HKS file not found: " + hksPath, LOG_DEBUG, "Cleanup");
            return;
        }

        auto lines = readFileLines(hksPath);
        if (lines.empty()) {
            log("Unable to read HKS file content", LOG_ERROR, "Cleanup");
            return;
        }

        log("==========================================", LOG_DEBUG, "Cleanup");
        log("HKS FILE DEBUG ANALYSIS", LOG_DEBUG, "Cleanup");
        log("File: " + hksPath, LOG_DEBUG, "Cleanup");
        log("Total lines: " + std::to_string(lines.size()), LOG_DEBUG, "Cleanup");
        log("==========================================", LOG_DEBUG, "Cleanup");

        // Look for any lines containing "LuaLoader" or "Lua Loader"
        bool foundAnyInjection = false;
        for (int i = 0; i < lines.size(); ++i) {
            const std::string& line = lines[i];
            if (line.find("LuaLoader") != std::string::npos ||
                line.find("Lua Loader") != std::string::npos ||
                line.find("module_loader") != std::string::npos) {

                log("Line " + std::to_string(i + 1) + ": " + line, LOG_DEBUG, "Cleanup");
                foundAnyInjection = true;
            }
        }

        if (!foundAnyInjection) {
            log("No LuaLoader-related content found in HKS file", LOG_DEBUG, "Cleanup");

            // Show first and last 5 lines for context
            log("First 5 lines:", LOG_TRACE, "Cleanup");
            for (int i = 0; i < std::min(5, static_cast<int>(lines.size())); ++i) {
                log("Line " + std::to_string(i + 1) + ": " + lines[i], LOG_TRACE, "Cleanup");
            }

            if (lines.size() > 10) {
                log("Last 5 lines:", LOG_TRACE, "Cleanup");
                for (int i = std::max(0, static_cast<int>(lines.size()) - 5); i < lines.size(); ++i) {
                    log("Line " + std::to_string(i + 1) + ": " + lines[i], LOG_TRACE, "Cleanup");
                }
            }
        }

        log("==========================================", LOG_DEBUG, "Cleanup");
    }

} // namespace Cleanup