cmake_minimum_required(VERSION 3.20)
project(LuaLoader)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    LuaLoader.cpp
    Console.cpp
    ConfigParser.cpp
    ConfigGenerator.cpp
    Me3Utils.cpp
    FlagFile.cpp
    HksInjector.cpp
    Logger.cpp
    LuaSetup.cpp
    PathUtils.cpp
)

# Add header files
set(HEADERS
    Console.h
    ConfigParser.h
    ConfigGenerator.h
    Me3Utils.h
    FlagFile.h
    HksInjector.h
    Logger.h
    LuaSetup.h
    PathUtils.h
)

# Create DLL
add_library(LuaLoader SHARED ${SOURCES} ${HEADERS})

# Link required Windows libraries
target_link_libraries(LuaLoader PRIVATE kernel32)

# Set output name
set_target_properties(LuaLoader PROPERTIES OUTPUT_NAME "LuaLoader")

# Export all symbols (for DLL)
set_target_properties(LuaLoader PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

# Optional: Set build configurations
if(MSVC)
    # Use static runtime
    set_property(TARGET LuaLoader PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Add compile options
    target_compile_options(LuaLoader PRIVATE /W4)
    
    # Optimize for release
    target_compile_options(LuaLoader PRIVATE $<$<CONFIG:Release>:/O2>)
endif()