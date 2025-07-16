// =============================================
// File: LuaSetup.cpp
// Category: Lua Setup Script Generation
// Purpose: Implements Lua setup script generation for module loading.
// =============================================
#include "LuaSetup.h"
#include "Logger.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Helper: string replace
static std::string replaceAll(std::string subject, const std::string& search, const std::string& replace) {
    std::string::size_type pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

void createWorkingSetupScript(const LoaderConfig& config) {
    if (config.modulePath.absolutePath.empty()) {
        log("Module path is empty, cannot create setup script", LOG_ERROR, "LuaSetup");
        return;
    }

    std::string loaderDir = config.modulePath.absolutePath + "/_module_loader";
    std::string setupScript = loaderDir + "/module_loader_setup.lua";

    log("Creating setup script at: " + setupScript, LOG_DEBUG, "LuaSetup");

    // prepare directory
    try {
        fs::create_directories(loaderDir);
        log("Created loader directory: " + loaderDir, LOG_DEBUG, "LuaSetup");

        if (fs::exists(setupScript)) {
            fs::remove(setupScript);
            log("Removed existing setup script", LOG_DEBUG, "LuaSetup");
        }
    }
    catch (const std::exception& e) {
        log("Cannot create setup directory '" + loaderDir + "': " + std::string(e.what()), LOG_ERROR, "LuaSetup");
        return;
    }
    catch (...) {
        log("Cannot create setup directory '" + loaderDir + "': unknown error", LOG_ERROR, "LuaSetup");
        return;
    }

    // Enhanced Lua template with process ID tracking and better path information
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
print("Current Process ID: " .. getCurrentProcessId())
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

    // inject actual paths with enhanced information
    std::string lua = LUA_TEMPLATE;
    lua = replaceAll(lua, "${LOADER_DIR}", loaderDir);
    lua = replaceAll(lua, "${MODULE_PATH}", config.modulePath.absolutePath);
    lua = replaceAll(lua, "${CONFIG_DIR}", config.configDir);
    lua = replaceAll(lua, "${CONFIG_RELATIVE_PATH}", config.gameScriptPath.relativePath);
    lua = replaceAll(lua, "${MODULE_RELATIVE_PATH}", config.modulePath.relativePath);

    log("Injecting paths into setup script template", LOG_DEBUG, "LuaSetup");

    // write out the file
    std::ofstream out(setupScript);
    if (!out.is_open()) {
        log("Cannot write setup script to: " + setupScript, LOG_ERROR, "LuaSetup");
        return;
    }

    out << lua;
    out.close();

    log("Setup script created successfully: " + setupScript, LOG_INFO, "LuaSetup");
}