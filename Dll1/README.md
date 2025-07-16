# Lua Loader v11.3 - Modular Version

This is the modularized version of the Lua Loader DLL, broken down into logical components for better maintainability and organization.

## File Structure

### Core Components

- **ModuleLoader.cpp** - Main DLL entry point and orchestration
- **ConfigParser.cpp/h** - Parses .me3/TOML configuration files
- **PathUtils.cpp/h** - Path normalization and resolution utilities
- **Logger.cpp/h** - Logging functionality and silent mode control
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
cl /std:c++17 /LD /O2 /Fe:ModuleLoader.dll ModuleLoader.cpp ConfigParser.cpp FlagFile.cpp HksInjector.cpp Logger.cpp LuaSetup.cpp PathUtils.cpp kernel32.lib
```

## Module Dependencies

- **Windows.h** - Windows API functions
- **filesystem** - C++17 filesystem operations
- **fstream** - File I/O operations
- **ctime** - Timestamp generation
- **algorithm** - String manipulation

## Configuration Format (.me3 files)

```toml
# Example configuration
gameScriptPath = "mod/action/script"
modulePath = "mod/action/script/lua"
silent = false
```

## Module Interactions

1. **ModuleLoader** initializes the DLL and coordinates all operations
2. **PathUtils** finds config files and resolves relative paths
3. **ConfigParser** reads .me3 files and populates LoaderConfig
4. **Logger** handles all console output (respects silent mode)
5. **FlagFile** manages process-specific module loading flags
6. **LuaSetup** generates the Lua script that loads modules
7. **HksInjector** modifies c0000.hks to include the loader

## Key Features Preserved

- Enhanced path resolution with multiple fallback strategies
- Process ID tracking to prevent duplicate module loading
- Automatic backup creation before HKS modification
- Silent mode support
- Comprehensive error handling and logging

## Usage

The modular version functions identically to the original:

1. Place the DLL in your game directory
2. Create a .me3 configuration file
3. The loader will automatically find and process your config
4. Lua modules will be loaded when the game runs