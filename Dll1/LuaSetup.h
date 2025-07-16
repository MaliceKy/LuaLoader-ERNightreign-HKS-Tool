// =============================================
// File: LuaSetup.h
// Category: Lua Setup Script Generation
// Purpose: Declares function to create the Lua setup script for module loading.
// =============================================
#pragma once
#include <string>
#include "ConfigParser.h"

void createWorkingSetupScript(const LoaderConfig& config);