// ============================================ =
// File: Me3Utils.h
// Category: ME3 File Utilities
// Purpose: Declarations for .me3 file manipulation functions.
// =============================================
#pragma once
#include <string>

std::string toLower(const std::string & str);
void injectTomlPathToMe3(const std::string& me3Path, const std::string& tomlPath);
