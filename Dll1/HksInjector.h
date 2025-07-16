// =============================================
// File: HksInjector.h
// Category: HKS Script Integration
// Purpose: Declares injection of Lua loader into the target .hks file.
// =============================================
#pragma once
#include "ConfigParser.h"
#include <string>

// Main injection function
void injectIntoHksFile(const LoaderConfig& config);

// Universal HKS backup function with context support
bool createHksBackup(const std::string& hksPath, const LoaderConfig& config, const std::string& context);