// =============================================
// File: LuaSetup.cpp
// Category: Lua Setup Script Generation
// Purpose: Implements Lua setup script generation for module loading.
// =============================================
#include "LuaSetup.h"
#include "Logger.h"
#include "ErrorMessages.h"  // For beautiful error messages
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

// Helper: Safe string replacement with bounds checking
static std::string replaceAll(std::string subject, const std::string& search, const std::string& replace) {
    if (search.empty()) return subject;  // Prevent infinite loop

    std::string::size_type pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

// Validate configuration before proceeding
static bool validateConfiguration(const LoaderConfig& config) {
    std::string issue;

    if (config.modulePath.absolutePath.empty()) {
        issue = "Module path is empty";
    }
    else if (config.configDir.empty()) {
        issue = "Config directory is empty";
    }
    else if (config.modulePath.absolutePath.find_first_not_of(" \t\r\n") == std::string::npos) {
        issue = "Module path contains only whitespace";
    }

    if (!issue.empty()) {
        log(ErrorMessages::formatLuaSetupConfigError(issue), LOG_BRAND);
        return false;
    }

    log("Configuration validation passed", LOG_DEBUG, "LuaSetup");
    return true;
}

// Create the loader directory with proper error handling
static bool createLoaderDirectory(const std::string& loaderDir) {
    try {
        if (fs::exists(loaderDir)) {
            log("Loader directory already exists: " + loaderDir, LOG_DEBUG, "LuaSetup");
        }
        else {
            fs::create_directories(loaderDir);
            log("Created loader directory: " + loaderDir, LOG_DEBUG, "LuaSetup");
        }
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        log(ErrorMessages::formatLuaSetupDirectoryError(loaderDir, "Filesystem error: " + std::string(e.what())), LOG_BRAND);
        return false;
    }
    catch (const std::exception& e) {
        log(ErrorMessages::formatLuaSetupDirectoryError(loaderDir, "System error: " + std::string(e.what())), LOG_BRAND);
        return false;
    }
    catch (...) {
        log(ErrorMessages::formatLuaSetupDirectoryError(loaderDir, "Unknown error occurred"), LOG_BRAND);
        return false;
    }
}

// Remove existing setup script if present
static bool cleanupExistingScript(const std::string& setupScript) {
    try {
        if (fs::exists(setupScript)) {
            fs::remove(setupScript);
            log("Removed existing setup script", LOG_DEBUG, "LuaSetup");
        }
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        log("Filesystem error removing existing script: " + std::string(e.what()), LOG_WARNING, "LuaSetup");
        return false;  // Continue anyway, might overwrite
    }
    catch (...) {
        log("Unknown error removing existing script", LOG_WARNING, "LuaSetup");
        return false;  // Continue anyway
    }
}

// Generate the Lua template with all substitutions
static std::string generateLuaScript(const LoaderConfig& config, const std::string& loaderDir) {
    static const char* LUA_TEMPLATE = R"LUASCRIPT(
-- Lua Loader by Malice - Setup Script (Enhanced Path Resolution Version)
local MODULE_PATH = "${MODULE_PATH}"
local LOADER_DIR = "${LOADER_DIR}"
local FLAG_FILE = LOADER_DIR .. "/.modules_loaded"
local CONFIG_DIR = "${CONFIG_DIR}"

function consolePrint(msg)
    local f = io.open("CONOUT$", "a")
    if f then f:write("  "..tostring(msg).."\n"); f:close() end
end
print = consolePrint

-- Get current process ID (Windows specific)
local function getCurrentProcessId()
    local handle = io.popen("echo %WINPID% 2>nul || powershell -Command \"Get-Process -Id $PID | Select-Object -ExpandProperty Id\"")
    local pid = "unknown"
    if handle then
        pid = handle:read("*l") or "unknown"
        handle:close()
    end
    return tostring(pid):gsub("%s+", "") -- trim whitespace
end

-- Check if modules are already loaded for this process
local function isAlreadyLoaded()
    local f = io.open(FLAG_FILE, "r")
    if not f then return false end
    
    local content = f:read("*a")
    f:close()
    
    if not content then return false end
    
    -- Look for current process ID in the flag file
    local currentPid = getCurrentProcessId()
    if content:find("PID:" .. currentPid) then
        return true
    end
    
    return false
end

-- Early exit if already loaded in this process
if isAlreadyLoaded() then
    print("Modules already loaded for this process - skipping")
    return
end

-- Header with enhanced path information
print("==========================================")
print("Module Loader - Enhanced Path Resolution Version")
print("Config directory: " .. CONFIG_DIR)
print("Module path (absolute): " .. MODULE_PATH)
print("Relative paths resolved from: ${CONFIG_RELATIVE_PATH}")
print("Module path (relative): ${MODULE_RELATIVE_PATH}")
print("==========================================")
print("")

-- Scan for .lua modules
local function scanForModules()
    local modules = {}
    local handle = io.popen('dir "' .. MODULE_PATH .. '\\*.lua" /b 2>nul')
    if handle then
        for filename in handle:lines() do
            local name = filename:match("(.+)%.lua$")
            if name and name ~= "module_loader_setup" then
                table.insert(modules, name)
            end
        end
        handle:close()
    end
    return modules
end

-- Main module loading function
function loadModules()
    -- Add module path to package.path
    package.path = package.path .. ";" .. MODULE_PATH .. "/?.lua"
    
    local modules = scanForModules()
    if #modules == 0 then
        print("No modules found in: " .. MODULE_PATH)
        return false
    end

    -- List modules to be loaded
    print("Loading " .. #modules .. "/" .. #modules .. " Modules:")
    for i, moduleName in ipairs(modules) do
        print("  " .. i .. ". " .. moduleName .. ".lua")
    end
    print("")

    -- Load each module
    local loadedCount = 0
    for _, moduleName in ipairs(modules) do
        local success, result = pcall(require, moduleName)
        if success then
            -- If module returns a table, make it globally available
            if type(result) == "table" then
                _G[moduleName] = result
            end
            loadedCount = loadedCount + 1
            print("  [OK] Loaded: " .. moduleName)
        else
            print("  [ERROR] Failed to load: " .. moduleName .. " - " .. tostring(result))
        end
    end

    -- Create flag file with process ID to prevent reloading
    local flagFile = io.open(FLAG_FILE, "w")
    if flagFile then
        flagFile:write("Loaded at: " .. os.date() .. "\n")
        flagFile:write("PID:" .. getCurrentProcessId() .. "\n")
        flagFile:write("Modules loaded: " .. loadedCount .. "/" .. #modules .. "\n")
        flagFile:write("Config directory: " .. CONFIG_DIR .. "\n")
        flagFile:write("Module path (absolute): " .. MODULE_PATH .. "\n")
        flagFile:write("Module path (relative): ${MODULE_RELATIVE_PATH}\n")
        flagFile:close()
    end

    print("")
    if loadedCount > 0 then
        print("[OK] " .. loadedCount .. "/" .. #modules .. " modules loaded successfully")
        print("==========================================")
        return true
    else
        print("[ERROR] No modules loaded successfully")
        print("==========================================")
        return false
    end
end

-- Execute the loading
loadModules()
)LUASCRIPT";

    // Perform all path substitutions
    std::string lua = LUA_TEMPLATE;
    lua = replaceAll(lua, "${LOADER_DIR}", loaderDir);
    lua = replaceAll(lua, "${MODULE_PATH}", config.modulePath.absolutePath);
    lua = replaceAll(lua, "${CONFIG_DIR}", config.configDir);
    lua = replaceAll(lua, "${CONFIG_RELATIVE_PATH}", config.gameScriptPath.relativePath);
    lua = replaceAll(lua, "${MODULE_RELATIVE_PATH}", config.modulePath.relativePath);

    log("Applied all path substitutions to Lua template", LOG_DEBUG, "LuaSetup");
    return lua;
}

// Write the script file with comprehensive error handling
static bool writeScriptFile(const std::string& setupScript, const std::string& luaContent) {
    try {
        std::ofstream out(setupScript, std::ios::binary);  // Use binary mode for consistency
        if (!out.is_open()) {
            log(ErrorMessages::formatLuaSetupScriptWriteError(setupScript, "Unable to open file for writing"), LOG_BRAND);
            return false;
        }

        out << luaContent;

        // Check for write errors
        if (out.bad()) {
            out.close();
            log(ErrorMessages::formatLuaSetupScriptWriteError(setupScript, "Write operation failed"), LOG_BRAND);
            return false;
        }

        // Flush and close with error checking
        out.flush();
        if (out.bad()) {
            out.close();
            log(ErrorMessages::formatLuaSetupScriptWriteError(setupScript, "Flush operation failed"), LOG_BRAND);
            return false;
        }

        out.close();
        log("Setup script written successfully", LOG_DEBUG, "LuaSetup");
        return true;

    }
    catch (const std::ios_base::failure& e) {
        log(ErrorMessages::formatLuaSetupScriptWriteError(setupScript, "I/O error: " + std::string(e.what())), LOG_BRAND);
        return false;
    }
    catch (const std::exception& e) {
        log(ErrorMessages::formatLuaSetupScriptWriteError(setupScript, "Write error: " + std::string(e.what())), LOG_BRAND);
        return false;
    }
    catch (...) {
        log(ErrorMessages::formatLuaSetupScriptWriteError(setupScript, "Unknown error occurred during write operation"), LOG_BRAND);
        return false;
    }
}

// Main function - now clean and organized
void createWorkingSetupScript(const LoaderConfig& config) {
    log("Starting setup script creation", LOG_DEBUG, "LuaSetup");

    // Step 1: Validate configuration
    if (!validateConfiguration(config)) {
        log("Setup script creation aborted due to configuration issues", LOG_ERROR, "LuaSetup");
        return;
    }

    // Step 2: Determine paths
    std::string loaderDir = config.modulePath.absolutePath + "/_module_loader";
    std::string setupScript = loaderDir + "/module_loader_setup.lua";

    log("Target setup script: " + setupScript, LOG_DEBUG, "LuaSetup");

    // Step 3: Create loader directory
    if (!createLoaderDirectory(loaderDir)) {
        log("Setup script creation aborted due to directory creation failure", LOG_ERROR, "LuaSetup");
        return;
    }

    // Step 4: Clean up existing script (non-fatal if fails)
    cleanupExistingScript(setupScript);

    // Step 5: Generate Lua script content
    log("Generating Lua script content", LOG_DEBUG, "LuaSetup");
    std::string luaContent = generateLuaScript(config, loaderDir);

    // Step 6: Write the script file
    if (!writeScriptFile(setupScript, luaContent)) {
        log("Setup script creation failed during file write operation", LOG_ERROR, "LuaSetup");
        return;
    }

    // Step 7: Success!
    log("Setup script created successfully: " + setupScript, LOG_INFO, "LuaSetup");
    log("Script size: " + std::to_string(luaContent.length()) + " bytes", LOG_DEBUG, "LuaSetup");
    log("Lua module loader is ready for operation", LOG_INFO, "LuaSetup");
}