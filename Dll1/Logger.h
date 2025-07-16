// =============================================
// File: Logger.h
// Category: Logging/Branding
// Purpose: Declares logging functions, log levels, and branding.
// =============================================
#pragma once
#include <string>

// Log levels with explicit values for bounds checking
enum LogLevel {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_BRAND = 5
};

// Log level management
void setLogLevel(LogLevel minLevel);
LogLevel getLogLevel();

// Silent mode (only errors and branding show)
void setSilentMode(bool silent);
bool isSilentMode();

// Main logging function
void log(const std::string& msg, LogLevel level = LOG_INFO, const char* source = nullptr);

// Branding banner
void logBranding();