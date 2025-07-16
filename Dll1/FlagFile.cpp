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
            if (!isSilentMode()) {
                log("Cleared module loaded flag for fresh reload");
            }
        }
    }
    catch (...) {
        // Silently continue if we can't remove the file
    }
}

void cleanupFlagFile(const std::string& modulePath) {
    if (modulePath.empty()) return;

    std::string flagFile = getFlagFilePath(modulePath);
    try {
        if (fs::exists(flagFile)) {
            fs::remove(flagFile);
            if (!isSilentMode()) {
                log("Cleanup: Removed flag file on process exit");
            }
        }
    }
    catch (...) {
        // Ignore cleanup errors
    }
}