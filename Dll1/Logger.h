// =============================================
// File: Logger.h
// Category: Logging/Branding
// Purpose: Declarations for logging functions, branding messages, and silent mode control.
// =============================================
#pragma once
#include <string>

enum LogLevel {
    LOG_INFO,
    LOG_OK,
    LOG_ERROR,
    LOG_BRAND,
    LOG_WARNING
};

void log(const std::string& msg, LogLevel level = LOG_INFO);
void logBranding();
void setSilentMode(bool silent);
bool isSilentMode();