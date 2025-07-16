// =============================================
// File: Logger.h
// Category: Logging/Branding
// Purpose: Declarations for logging functions, branding messages, and silent mode control.
// =============================================
#pragma once
#include <string>

// Log levels (lower = more verbose, higher = more severe)
enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_BRAND
};

// Set the minimum level that will be logged (default: LOG_INFO).
void setLogLevel(LogLevel minLevel);
LogLevel getLogLevel();

// Set silent mode (forces errors/branding only, for compatibility with your old 'silent')
void setSilentMode(bool silent);
bool isSilentMode();

// Main log function. Example: log("some msg", LOG_DEBUG, "ConfigParser")
void log(const std::string& msg, LogLevel level = LOG_INFO, const char* source = nullptr);

// Print the loader banner (branding)
void logBranding();
