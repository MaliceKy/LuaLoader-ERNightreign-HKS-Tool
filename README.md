# LuaLoader – Modular Lua for ME3 (Elden Ring Nightreign)

**LuaLoader by Malice** - brings plug-and-play modular Lua scripting to mod Engine 3 (Elden Ring Nightreign), with bulletproof injection, smart backups, debug console, and a one-click cleanup for shipping your mod to others with ease.

---

## Table of Contents

* [What is LuaLoader?](#what-is-lualoader)
* [How It Works](#how-it-works)
* [Features](#features)
* [Installation & Usage](#installation--usage)
* [Configuration (`LuaLoader.toml`)](#configuration-lualoadertoml)
* [How to Clean Up & Ship Your Mod](#how-to-clean-up--ship-your-mod)
* [File-by-File Overview](#file-by-file-overview)
* [Development & Debugging](#development--debugging)
* [FAQ / Gotchas](#faq--gotchas)
* [License](#license)

---

## What is LuaLoader?

**LuaLoader** lets you distribute and run modular Lua scripts alongside your mod’s assets, directly from within your own folders, using a simple TOML config to handle paths, logging, backup, and cleanup. No hardcoded paths, no messing with other mods. When you're ready to ship, one flag (`cleanupOnNextLaunch`) erases all loader artifacts, resets everything, and leaves just your Lua scripts and assets—clean, portable, ready to upload.

---

## How It Works

1. **Injection:** The loader DLL injects a header and a call to a generated Lua script into your `c0000.hks`, setting up your Lua environment.
2. **Config:** Reads `LuaLoader.toml` for paths, logging, backup settings, and more.
3. **Modularity:** Loads every `.lua` file in your module folder (excluding the setup script itself) and makes tables globally available.
4. **Flagging:** Uses a `.modules_loaded` file to track module load state (per process).
5. **Cleanup:** On request, erases the loader’s traces (scripts, flags, injection), **restores HKS**, and leaves everything ready to upload or ship.

---

## Features

* **Fully modular:** Place all your Lua scripts in any directory, set paths relatively in TOML.
* **Automatic path resolution:** Relative to `.me3`, current working directory, or wherever you need.
* **Auto-generated config:** If missing, LuaLoader writes out a complete `LuaLoader.toml` with clear instructions.
* **Safe HKS backup:** Never lose your `c0000.hks`—backups are auto-created on every injection (or every launch, if configured).
* **Verbose logging:** Debug, trace, info, warning, and error logs, all configurable.
* **Debug Console:** Pops up a console for instant script output and debugging.
* **Easy distribution:** After cleanup, your mod directory contains only what you need—no loader junk.
* **CI/CD friendly:** Preconfigure TOMLs, run cleanup, and your mod’s zip is ready for Nexus/Thunderstore/anywhere.

---

## Installation & Usage

1. **Copy the DLL:** Drop `LuaLoader.dll` anywhere up to three directories above your `.me3` file.
2. **Edit or create your `.me3` config** (the loader will scan standard locations and generate `LuaLoader.toml` if missing).
3. **Set up your folder structure:**

   ```
   YourMod/
      mod/
         action/
            script/
               c0000.hks
               lua/
                  mymodule1.lua
                  mymodule2.lua
   ```
4. **Launch your game/mod with the DLL loaded.**
5. **First run:** Loader injects itself, creates configs/scripts, and logs everything to the console.

---

## Configuration (`LuaLoader.toml`)

Here’s what the auto-generated TOML looks like, with some explanation:

```toml
# LuaLoader.toml
configVersion = 1

# Path to your main HKS scripts (relative or absolute)
gameScriptPath = "mod/action/script"

# Path to Lua modules (relative to .me3 or absolute, optional)
modulePath = "mod/action/script/lua"

# Logging level: trace | debug | info | warning | error
logLevel = "info"

# HKS backup settings
backupHKSonLaunch = false        # true = backup HKS every launch, false = only when injecting
backupHKSFolder = "HKS-Backups"  # Where backups are saved

# CLEANUP: set true to remove loader artifacts and restore everything for shipping
cleanupOnNextLaunch = false
```

**All paths** can be relative to the `.me3` file or absolute. Forward slashes or double backslashes work. Spaces are supported.

---

## How to Clean Up & Ship Your Mod

**Ready to package and upload?**
Set `cleanupOnNextLaunch = true` in `LuaLoader.toml` and launch the game/mod once.

**What happens:**

* `_module_loader/` directory is deleted
* All `.modules_loaded` flags are removed
* LuaLoader code is stripped out of `c0000.hks` (original is restored from backup)
* Your TOML flag resets to `false`
* You’re left with only your scripts/assets and HKS backup—ready to zip/upload anywhere

**Your users:**
All they need to do is select the `.me3` file and launch with the DLL. Relative paths mean no configuration is required.

---

## File-by-File Overview

Here’s what the codebase actually does, **file by file** (high-level, but tells you what’s important):

* **ConfigGenerator.cpp:** Writes a full `LuaLoader.toml` with all supported settings and instructions.
* **ConfigParser.cpp:** Parses the TOML and `.me3` config files, handles path/flag logic, supports overrides, and validates all required settings.
* **LuaLoader.cpp:** Orchestrates everything. Scans for configs, initializes paths, injects the loader, handles cleanup, and logs branding.
* **LuaSetup.cpp:** Generates the actual Lua bootstrap (`module_loader_setup.lua`)—loads every `.lua` file in your modules folder, prints output to the debug console, and writes a flag to prevent redundant loading.
* **HksInjector.cpp:** Handles injection of loader code into `c0000.hks`, with robust backup and header, and makes sure no duplicate injections happen.
* **Cleanup.cpp:** Does what it says—erases `_module_loader`, flag files, and LuaLoader’s HKS injection; restores HKS from backup.
* **Logger.cpp:** Prints color-coded logs to console, supports silent mode, and shows a branding banner.
* **Console.h/cpp:** Allocates/releases a Windows console for debug prints and script output.
* **FlagFile.cpp:** Creates, removes, and manages `.modules_loaded` flags for module load state.

---

## Development & Debugging

* **Debug output** is sent to the dedicated console window.
* Use `logLevel = "trace"` for everything, or `"info"` for normal use.
* All operations (inject, cleanup, backup, flag file, etc.) are logged.
* If you want silent mode (only errors), set `logLevel = "error"` or use silent mode.

---

## Example Distribution Flow

1. Develop/test with modular scripts and loader DLL present.
2. Set relative paths in your TOML.
3. When ready, set `cleanupOnNextLaunch = true` and run the game once.
4. Zip/upload your cleaned mod directory.
5. Tell users to select the `.me3` file—done.

---

## Why LuaLoader?

Because it makes scripting the HKS a little more barable:

* You keep your mod clean, modular, and easy to read and maintain.
* Lua files do not need to be set in the game directory instead can stay in the mod engine folder.
* Shipping mods is literally one click away if paths are relatively set in the toml.

