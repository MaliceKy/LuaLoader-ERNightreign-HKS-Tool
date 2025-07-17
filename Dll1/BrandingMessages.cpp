// =============================================
// File: BrandingMessages.cpp
// Category: Branding & Visual Identity
// Purpose: Implements beautiful branding message formatting functions.
// =============================================
#include "BrandingMessages.h"

namespace BrandingMessages {

    std::string formatMainBranding() {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                        LUA LOADER BY MALICE                    \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  VERSION: v1.0.0 - Basic Tool Functionalities\n"
            "\n"
            "  ┌─ FEATURES ─────────────────────────────────────────────────────┐\n"
            "   • Professional Lua module loading system\n"
            "   • Intelligent path resolution and error handling\n"
            "   • Automatic HKS injection and backup management\n"
            "   • Process-aware module loading with duplicate prevention\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  Ready for game modding operations.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatInitBranding() {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                       INITIALIZATION STARTED                   \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  Starting Lua Loader v11.3 initialization sequence...\n"
            "  Preparing module loading system for operation.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatSuccessBranding() {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                     INITIALIZATION COMPLETE                    \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  ┌─ SYSTEM STATUS ────────────────────────────────────────────────┐\n"
            "   [✓] Configuration loaded and validated\n"
            "   [✓] Paths resolved and accessible\n"
            "   [✓] Module loader scripts prepared\n"
            "   [✓] HKS injection completed successfully\n"
            "   [✓] Ready for module loading operations\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  Lua Loader v11.3 is now ready for operation.\n"
            "  Modules will be loaded when the game script executes.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

    std::string formatErrorBranding() {
        return "\n"
            "  ╔═══════════════════════════════════════════════════════════════╗\n"
            "                      INITIALIZATION FAILED                     \n"
            "  ╚═══════════════════════════════════════════════════════════════╝\n"
            "\n"
            "  ┌─ SYSTEM STATUS ────────────────────────────────────────────────┐\n"
            "   [✗] Critical error occurred during initialization\n"
            "   [✗] Lua Loader cannot continue operation\n"
            "   [✗] Please review error messages above for details\n"
            "  └────────────────────────────────────────────────────────────────┘\n"
            "\n"
            "  Please fix the issues and restart the application.\n"
            "  Refer to the troubleshooting guide for assistance.\n"
            "  ═══════════════════════════════════════════════════════════════\n";
    }

}