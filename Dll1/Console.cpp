#include "Console.h"
#include <windows.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>

void InitConsole() {
    AllocConsole();
    SetConsoleTitle(L"Elden Ring Nightreign - Debug Console");

    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    std::ios::sync_with_stdio();

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    std::cout << "========================================" << std::endl;
    std::cout << "  ELDEN RING NIGHTREIGN DEBUG CONSOLE  " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Debug console initialized successfully!" << std::endl;
    std::cout << "HKS scripts can now print to this window." << std::endl;
    std::cout << "========================================" << std::endl;
}

void FreeConsoleOnExit() {
    FreeConsole();
}
