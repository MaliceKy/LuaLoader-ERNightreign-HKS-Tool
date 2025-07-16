// =============================================
// File: Logger.cpp
// Category: Logging/Branding
// Purpose: Implements logging output, branding banner, and silent mode logic.
// =============================================
#include "Logger.h"
#include <cstdio>
#include <ctime>
#include <mutex>
#include <windows.h>
#include <string>
#include <algorithm>

static LogLevel g_minLogLevel = LOG_INFO;
static bool g_silentMode = false;
static std::mutex g_logMutex;

static const char* levelNames[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "BRAND"
};

// Safe way to get level name with bounds checking
static const char* getSafeLevelName(LogLevel level) {
    if (level >= 0 && level < (sizeof(levelNames) / sizeof(levelNames[0]))) {
        return levelNames[level];
    }
    return "UNKNOWN";
}

void setLogLevel(LogLevel minLevel) {
    g_minLogLevel = minLevel;
}

LogLevel getLogLevel() {
    return g_minLogLevel;
}

void setSilentMode(bool silent) {
    g_silentMode = silent;
    // If silent, only errors and branding will show.
    if (silent) setLogLevel(LOG_ERROR);
}

bool isSilentMode() {
    return g_silentMode;
}

static std::string getTimeString() {
    char buf[16] = {};
    std::time_t t = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &t);
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    return std::string(buf);
}

void log(const std::string& msg, LogLevel level, const char* source) {
    if (g_silentMode && level != LOG_ERROR && level != LOG_BRAND)
        return;
    if (level < g_minLogLevel && level != LOG_ERROR && level != LOG_BRAND)
        return;

    std::lock_guard<std::mutex> lock(g_logMutex);
    FILE* f = nullptr;
    if (fopen_s(&f, "CONOUT$", "a") == 0 && f) {
        if (level == LOG_BRAND) {
            fprintf(f, "%s", msg.c_str());
        }
        else {
            std::string timeStr = getTimeString();
            // Format: [HH:MM:SS] [LEVEL][Source] message
            fprintf(
                f, "[%s] [%s]%s%s%s %s\n",
                timeStr.c_str(),
                getSafeLevelName(level),  // Use safe bounds-checked function
                (source ? " [" : ""),
                (source ? source : ""),
                (source ? "]" : ""),
                msg.c_str()
            );
        }
        fflush(f);
        fclose(f);
    }
}

void logBranding() {
    log("\n"
        "  ==========================================\n"
        "            Lua Loader by Malice\n"
        "      v11.3 - Enhanced Path Resolution\n"
        "  ==========================================\n\n", LOG_BRAND);
}