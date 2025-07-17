// =============================================
// File: BrandingMessages.h
// Category: Branding & Visual Identity
// Purpose: Provides beautiful branding message formatting functions.
// =============================================
#pragma once
#include <string>

namespace BrandingMessages {

    // Main branding banner
    std::string formatMainBranding();

    // Success completion banner
    std::string formatSuccessBranding();

    // Initialization start banner
    std::string formatInitBranding();

    // Error state banner (for critical failures)
    std::string formatErrorBranding();

}