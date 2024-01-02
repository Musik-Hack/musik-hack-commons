# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.
cmake_minimum_required(VERSION 3.2)
project(JUCE_PLUGINS VERSION 0.0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(MUSIKHACK_SAMPLES_DIR "${CMAKE_SOURCE_DIR}/sounds")

add_subdirectory(deps/JUCE)


# Modules to build
juce_add_module("modules/musikhack/lockfree" ALIAS_NAMESPACE musikhack)

include_directories("modules")

# Plugins to build
add_subdirectory("plugins/examples/LockfreeExample")
