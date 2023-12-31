# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.
cmake_minimum_required(VERSION 3.2)
project(JUCE_CMAKE_PROJECT VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_subdirectory(deps/JUCE)


# Modules to build
juce_add_module("modules/musikhack/lockfree" ALIAS_NAMESPACE musikhack)

# Plugins to build
add_subdirectory("plugins/examples/LockfreeExample")
