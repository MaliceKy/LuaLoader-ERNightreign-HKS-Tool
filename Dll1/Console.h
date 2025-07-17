// =============================================
// File: Console.h
// Category: Console Management
// Purpose: Declares console initialization and cleanup functions.
// =============================================
#pragma once

// Initialize debug console with UTF-8 support and stream redirection
void InitConsole();

// Display the console initialization banner
void displayConsoleBanner();

// Clean up and free the console on application exit
void FreeConsoleOnExit();