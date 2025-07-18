// =============================================
// File: PathUtils.h
// Category: Filesystem Utilities
// Purpose: Declares helpers for normalizing and resolving file system paths.
// =============================================
#pragma once
#include <string>
#include <vector>
#include <filesystem>

// Forward declaration to avoid circular dependency
struct LoaderConfig;

std::string normalizePath(const std::string& path);
std::string resolvePathWithFallbacks(const std::string& inputPath, const std::string& configDir);
std::vector<std::string> findConfigFiles(const std::filesystem::path& searchPath, int maxDepth = 3);
bool validatePaths(LoaderConfig& config);
