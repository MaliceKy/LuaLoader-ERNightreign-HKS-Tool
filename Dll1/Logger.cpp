// =============================================
// File: Logger.cpp
// Category: Logging/Branding
// Purpose: Implements logging output, branding banner, and silent mode logic.
// =============================================
#include "Logger.h"
#include <cstdio>

static bool g_silentMode = false;

void setSilentMode(bool silent) {
    g_silentMode = silent;
}

bool isSilentMode() {
    return g_silentMode;
}

void log(const std::string& msg, LogLevel level) {
    if (g_silentMode && level != LOG_ERROR) return;

    FILE* f = nullptr;
    if (fopen_s(&f, "CONOUT$", "a") == 0 && f) {
        switch (level) {
        case LOG_OK:
            fprintf(f, "  [OK] %s\n", msg.c_str());
            break;
        case LOG_ERROR:
            fprintf(f, "  [ERROR] %s\n", msg.c_str());
            break;
        case LOG_BRAND:
            fprintf(f, "%s", msg.c_str());
            break;
        default:
            fprintf(f, "  %s\n", msg.c_str());
            break;
        }
        fflush(f);
        fclose(f);
    }
}

void logBranding() {
    FILE* f = nullptr;
    if (fopen_s(&f, "CONOUT$", "a") == 0 && f) {
        fprintf(f, "\n");
        fprintf(f, "  ==========================================\n");
        fprintf(f, "            Lua Loader by Malice\n");
        fprintf(f, "      v11.3 - Enhanced Path Resolution\n");
        fprintf(f, "  ==========================================\n");
        fprintf(f, "\n");
        fflush(f);
        fclose(f);
    }
}