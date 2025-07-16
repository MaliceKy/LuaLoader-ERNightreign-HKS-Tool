// =============================================
// File: FlagFile.h
// Category: Module Load Tracking
// Purpose: Declares helpers for flag file creation/removal (.modules_loaded).
// =============================================
#pragma once
#include <string>

std::string getFlagFilePath(const std::string& modulePath);
void clearModuleLoadedFlag(const std::string& modulePath);
void cleanupFlagFile(const std::string& modulePath);