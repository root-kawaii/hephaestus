# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.27.6/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.27.6/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan

# Include any dependencies generated for this target.
include third_party/CMakeFiles/bullet3.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include third_party/CMakeFiles/bullet3.dir/compiler_depend.make

# Include the progress variables for this target.
include third_party/CMakeFiles/bullet3.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/CMakeFiles/bullet3.dir/flags.make

third_party/CMakeFiles/bullet3.dir/clean:
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan/third_party && $(CMAKE_COMMAND) -P CMakeFiles/bullet3.dir/cmake_clean.cmake
.PHONY : third_party/CMakeFiles/bullet3.dir/clean

third_party/CMakeFiles/bullet3.dir/depend:
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan/third_party /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan/third_party /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan/third_party/CMakeFiles/bullet3.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : third_party/CMakeFiles/bullet3.dir/depend

