# Lua Loader - Modular Version

This is the modularized version of the Lua Loader DLL, broken down into logical components for better maintainability and organization.

## File Structure

### Core Components
- **LuaLoader.cpp** - Main DLL entry point and orchestration
- **ConfigParser.cpp/h** - Parses .me3/TOML configuration files
- **ConfigGenerator.cpp/h** - Generates default TOML config files
- **Me3Utils.cpp/h** - Utilities for manipulating .me3 files
- **PathUtils.cpp/h** - Path normalization, resolution utilities, and validation
- **Logger.cpp/h** - Logging functionality, branding, and silent mode control
- **FlagFile.cpp/h** - Module load tracking with flag files
- **LuaSetup.cpp/h** - Generates the Lua setup script
- **HksInjector.cpp/h** - Injects the loader into c0000.hks

## Building

### Using CMake (Recommended)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Using Visual Studio
1. Create a new DLL project
2. Add all .cpp and .h files to the project
3. Set C++ standard to C++17 or higher
4. Build in Release mode

### Using Command Line (MSVC)
```bash
cl /std:c++17 /LD /O2 /Fe:LuaLoader.dll LuaLoader.cpp ConfigParser.cpp ConfigGenerator.cpp Me3Utils.cpp FlagFile.cpp HksInjector.cpp Logger.cpp LuaSetup.cpp PathUtils.cpp kernel32.lib
```

## Module Dependencies
- **Windows.h** - Windows API functions
- **filesystem** - C++17 filesystem operations
- **fstream** - File I/O operations
- **ctime** - Timestamp generation
- **algorithm** - String manipulation

## Configuration Format

### .me3 files
```toml
# Optional: Custom config path override
luaLoaderConfigPath = "path/to/custom/LuaLoader.toml"
```

### LuaLoader.toml files
```toml
# Required configuration
configVersion = 1
gameScriptPath = "mod/action/script"
modulePath = "mod/action/script/lua"

# Optional settings
silent = false
backupHKSonLaunch = true
backupHKSFolder = "HKS-Backups"
```

## Module Interactions

1. **LuaLoader** initializes the DLL and coordinates all operations
2. **ConfigParser** reads .me3 files to find config paths and parses TOML configs
3. **ConfigGenerator** creates default TOML configurations when none exist
4. **Me3Utils** handles injection of config paths into .me3 files
5. **PathUtils** resolves paths with fallback strategies and validates directories
6. **Logger** handles all console output with branding (respects silent mode)
7. **FlagFile** manages process-specific module loading flags
8. **LuaSetup** generates the Lua script that loads modules
9. **HksInjector** modifies c0000.hks to include the loader

## Key Features Preserved

- **Enhanced path resolution** with multiple fallback strategies
- **Automatic config generation** with user-friendly defaults
- **Process ID tracking** to prevent duplicate module loading
- **Automatic backup creation** before HKS modification
- **Silent mode support** for clean operation
- **Comprehensive error handling and logging** with detailed feedback
- **Flexible config placement** - configs can be anywhere with path override

## Improved Organization Benefits

- **Single Responsibility** - Each file has one clear purpose
- **Better Maintainability** - Easier to find and modify specific functionality
- **Cleaner Architecture** - Main loader just orchestrates, doesn't implement details
- **Improved Testability** - Functions can be tested in isolation
- **Logical Grouping** - Related functions are organized together

## Usage

The modular version functions identically to the original:

1. **Place the DLL** in your game directory
2. **Create a .me3 configuration file** (minimal setup required)
3. **LuaLoader.toml will be auto-generated** with defaults
4. **Edit the TOML config** to set your paths and preferences  
5. **Relaunch the game** - Lua modules will load automatically

### First-Time Setup Process

1. Create basic .me3 file
2. Launch game - LuaLoader.toml is generated automatically  
3. Edit LuaLoader.toml with your specific paths
4. Relaunch game - everything works!

For custom config locations, add to your .me3 file:
```toml
luaLoaderConfigPath = "D:/My/Custom/Path/MyConfig.toml"
```
