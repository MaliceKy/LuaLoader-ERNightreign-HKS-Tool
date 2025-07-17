// =============================================
// File: ErrorMessages.h
// Category: Error Formatting
// Purpose: Provides clean, professional error message formatting functions.
// =============================================
#pragma once
#include <string>

// Forward declaration
struct LoaderConfig;

namespace ErrorMessages {

    // HKS-related error messages
    std::string formatHksNotFoundError(const std::string& hksPath, const LoaderConfig& config);
    std::string formatHksAccessError(const std::string& hksPath, const std::string& errorDetails);
    std::string formatHksSystemError(const std::string& hksPath);
    std::string formatHksWriteError(const std::string& hksPath, const std::string& errorDetails);
    std::string formatHksWriteSystemError(const std::string& hksPath);

    // Configuration-related error messages
    std::string formatEmptyGameScriptPathError(const std::string& configFile);
    std::string formatHksReadError(const std::string& hksPath);

    // ME3 - related error messages
    std::string formatMe3ReadError(const std::string & me3Path, const std::string & errorDetails);
    std::string formatMe3WriteError(const std::string& me3Path, const std::string& errorDetails);

    // LuaSetup-related error messages
    std::string formatLuaSetupConfigError(const std::string& issue);
    std::string formatLuaSetupDirectoryError(const std::string& loaderDir, const std::string& errorDetails);
    std::string formatLuaSetupScriptWriteError(const std::string& setupScript, const std::string& errorDetails);
}