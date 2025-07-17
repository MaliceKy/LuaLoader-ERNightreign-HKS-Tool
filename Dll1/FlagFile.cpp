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
    if (modulePath.empty()) {
        return "";
    }
    return modulePath + "/_module_loader/.modules_loaded";
}

void clearModuleLoadedFlag(const std::string& modulePath) {
    if (modulePath.empty()) {
        log("Cannot clear flag: modulePath is empty", LOG_WARNING, "FlagFile");
        return;
    }

    std::string flagFile = getFlagFilePath(modulePath);
    if (flagFile.empty()) {
        log("Cannot clear flag: invalid flag file path", LOG_WARNING, "FlagFile");
        return;
    }

    try {
        if (fs::exists(flagFile)) {
            fs::remove(flagFile);
            log("Cleared module loaded flag for fresh reload", LOG_DEBUG, "FlagFile");
        }
        else {
            log("Flag file does not exist, nothing to clear", LOG_TRACE, "FlagFile");
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        log("Filesystem error removing flag file: " + std::string(e.what()), LOG_WARNING, "FlagFile");
    }
    catch (const std::exception& e) {
        log("Error removing flag file: " + std::string(e.what()), LOG_WARNING, "FlagFile");
    }
    catch (...) {
        log("Unknown error removing flag file", LOG_WARNING, "FlagFile");
    }
}

void cleanupFlagFile(const std::string& modulePath) {
    if (modulePath.empty()) {
        log("Cannot cleanup flag: modulePath is empty", LOG_TRACE, "FlagFile");
        return;
    }

    std::string flagFile = getFlagFilePath(modulePath);
    if (flagFile.empty()) {
        log("Cannot cleanup flag: invalid flag file path", LOG_TRACE, "FlagFile");
        return;
    }

    try {
        if (fs::exists(flagFile)) {
            fs::remove(flagFile);
            log("Cleanup: Removed flag file on process exit", LOG_DEBUG, "FlagFile");
        }
        else {
            log("Cleanup: Flag file does not exist", LOG_TRACE, "FlagFile");
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        log("Cleanup filesystem error: " + std::string(e.what()), LOG_TRACE, "FlagFile");
    }
    catch (const std::exception& e) {
        log("Cleanup error: " + std::string(e.what()), LOG_TRACE, "FlagFile");
    }
    catch (...) {
        log("Cleanup unknown error", LOG_TRACE, "FlagFile");
    }
}