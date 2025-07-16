// =============================================
// File: FlagFile.cpp
// Category: Module Load Tracking
// Purpose: Implements helpers for creating/clearing module loaded flag file.
// =============================================
#include "FlagFile.h"
#include "Logger.h"
#include <filesystem>

namespace fs = std::filesystem;

std::string getFlagFilePath(const std::string& modulePath) {
    return modulePath + "/_module_loader/.modules_loaded";
}

void clearModuleLoadedFlag(const std::string& modulePath) {
    if (modulePath.empty()) return;

    std::string flagFile = getFlagFilePath(modulePath);
    try {
        if (fs::exists(flagFile)) {
            fs::remove(flagFile);
            log("Cleared module loaded flag for fresh reload", LOG_DEBUG, "FlagFile");
        }
    }
    catch (const std::exception& e) {
        log("Failed to remove module loaded flag: " + std::string(e.what()), LOG_WARNING, "FlagFile");
    }
    catch (...) {
        log("Failed to remove module loaded flag: unknown error", LOG_WARNING, "FlagFile");
    }
}

void cleanupFlagFile(const std::string& modulePath) {
    if (modulePath.empty()) return;

    std::string flagFile = getFlagFilePath(modulePath);
    try {
        if (fs::exists(flagFile)) {
            fs::remove(flagFile);
            log("Cleanup: Removed flag file on process exit", LOG_DEBUG, "FlagFile");
        }
    }
    catch (const std::exception& e) {
        log("Cleanup failed: " + std::string(e.what()), LOG_TRACE, "FlagFile");
    }
    catch (...) {
        log("Cleanup failed: unknown error", LOG_TRACE, "FlagFile");
    }
}