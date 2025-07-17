// =============================================
// File: Console.cpp
// Category: Console Management
// Purpose: Handles console initialization and setup for debug output.
// =============================================
#include "Console.h"
#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

void InitConsole() {
    // Allocate a new console for this process
    AllocConsole();
    SetConsoleTitle(L"Lua Loader by Malice - Debug Console");

    // Enable UTF-8 support for proper character encoding
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Enable Virtual Terminal Processing for better Unicode and color support
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    // Redirect standard streams to console
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);

    // Synchronize C++ streams with C stdio
    std::ios::sync_with_stdio(true);

    // Set console text attributes to default color
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    // Display console initialization banner
    displayConsoleBanner();
}

void displayConsoleBanner() {
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n"
        << "                     DEBUG CONSOLE READY                      \n"
        << "╚═══════════════════════════════════════════════════════════════╝\n"
        << "\n"
        << "Debug console initialized successfully!\n"
        << "Lua scripts and system messages will appear here.\n"
        << "═══════════════════════════════════════════════════════════════\n"
        << std::endl;
}

void FreeConsoleOnExit() {
    FreeConsole();
}