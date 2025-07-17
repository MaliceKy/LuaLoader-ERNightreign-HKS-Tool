// =============================================
// File: Logger.cpp
// Category: Logging/Branding
// Purpose: Implements logging output, branding banner, and silent mode logic.
// =============================================
#include "Logger.h"
#include "BrandingMessages.h"
#include <cstdio>
#include <ctime>
#include <mutex>
#include <windows.h>
#include <string>
#include <algorithm>

// Global logging state
static LogLevel g_minLogLevel = LOG_INFO;
static bool g_silentMode = false;
static std::mutex g_logMutex;

// Log level name mapping
static const char* levelNames[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "BRAND"
};

// Helper function to safely get level name with bounds checking
static const char* getSafeLevelName(LogLevel level) {
    const size_t numLevels = sizeof(levelNames) / sizeof(levelNames[0]);

    if (level >= 0 && level < static_cast<int>(numLevels)) {
        return levelNames[level];
    }
    return "UNKNOWN";
}

// Helper function to generate timestamp string
static std::string getTimeString() {
    char timeBuffer[16] = {};
    std::time_t currentTime = std::time(nullptr);
    std::tm timeStruct;

    if (localtime_s(&timeStruct, &currentTime) == 0) {
        std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeStruct);
    }

    return std::string(timeBuffer);
}

// Log level management functions
void setLogLevel(LogLevel minLevel) {
    g_minLogLevel = minLevel;
}

LogLevel getLogLevel() {
    return g_minLogLevel;
}

// Silent mode management functions
void setSilentMode(bool silent) {
    g_silentMode = silent;

    // In silent mode, only show errors and branding
    if (silent) {
        setLogLevel(LOG_ERROR);
    }
}

bool isSilentMode() {
    return g_silentMode;
}

// Core logging function
void log(const std::string& message, LogLevel level, const char* source) {
    // Filter messages based on silent mode and log level
    if (g_silentMode && level != LOG_ERROR && level != LOG_BRAND) {
        return;
    }

    if (level < g_minLogLevel && level != LOG_ERROR && level != LOG_BRAND) {
        return;
    }

    // Thread-safe logging
    std::lock_guard<std::mutex> lock(g_logMutex);

    FILE* consoleFile = nullptr;
    if (fopen_s(&consoleFile, "CONOUT$", "a") == 0 && consoleFile) {
        if (level == LOG_BRAND) {
            // Branding messages are printed as-is without formatting
            fprintf(consoleFile, "%s", message.c_str());
        }
        else {
            // Regular log messages with timestamp and level formatting
            std::string timestamp = getTimeString();

            fprintf(consoleFile, "[%s] [%s]%s%s%s %s\n",
                timestamp.c_str(),
                getSafeLevelName(level),
                (source ? " [" : ""),
                (source ? source : ""),
                (source ? "]" : ""),
                message.c_str()
            );
        }

        fflush(consoleFile);
        fclose(consoleFile);
    }
}

// Branding functions using the BrandingMessages system
void logBranding() {
    log(BrandingMessages::formatMainBranding(), LOG_BRAND);
}

void logInitBranding() {
    log(BrandingMessages::formatInitBranding(), LOG_BRAND);
}

void logSuccessBranding() {
    log(BrandingMessages::formatSuccessBranding(), LOG_BRAND);
}

void logErrorBranding() {
    log(BrandingMessages::formatErrorBranding(), LOG_BRAND);
}