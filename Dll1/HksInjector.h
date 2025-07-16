// =============================================
// File: HksInjector.h
// Category: HKS Script Integration
// Purpose: Declares function to inject Lua loader into the target .hks file.
// =============================================
#pragma once
#include "ConfigParser.h"

void injectIntoHksFile(const LoaderConfig& config);