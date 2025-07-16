// =============================================
// File: DefaultConfig.h
// Category: Config Generation
// Purpose: Declaration for default config TOML generator.
// =============================================
#pragma once
#include <string>

void generateDefaultConfigToml(const std::string& configPath);

void injectTomlPathToMe3(const std::string& me3Path, const std::string& tomlPath);
