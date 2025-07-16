// =============================================
// File: Cleanup.h
// Category: Cleanup Utilities
// Purpose: Declares cleanup functions for removing loader artifacts
// =============================================
#pragma once
#include "ConfigParser.h"
#include <string>

namespace Cleanup {
    // Main cleanup orchestrator - removes all loader-generated artifacts
    // Returns true if all operations succeeded, false if any warnings occurred
    bool performFullCleanup(LoaderConfig& config);

    // Removes the _module_loader directory and all its contents
    // Returns true on success or if directory doesn't exist
    bool cleanupModuleLoaderDirectory(const std::string& modulePath);

    // Removes .modules_loaded flag files from module and loader directories
    // Returns true if all flag files were successfully processed
    bool cleanupFlagFiles(const std::string& modulePath);

    // Removes LuaLoader injection block from HKS file (creates backup first)
    // Returns true on success or if no injection found
    bool cleanupHksInjection(const std::string& hksPath);

    // Debug function to analyze HKS file content
    void debugHksFile(const std::string& hksPath);
}