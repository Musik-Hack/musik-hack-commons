# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.
cmake_minimum_required(VERSION 3.2)
project(JUCE_PLUGINS VERSION 0.0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(MUSIKHACK_SAMPLES_DIR "${CMAKE_SOURCE_DIR}/samples")

# Add the JUCE library directory
add_subdirectory("deps/JUCE")

# Modules to build
add_subdirectory("modules/musikhack")

# Add the modules to the include path
include_directories("modules")

# Plugins to build
add_subdirectory("plugins/examples/LockfreeExample")
add_subdirectory("plugins/examples/MinimalExample")

# To make your own, copy the MinimalExample and rename it. Then add it to this list.
