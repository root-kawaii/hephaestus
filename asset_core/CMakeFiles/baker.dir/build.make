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
CMAKE_SOURCE_DIR = /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point

# Include any dependencies generated for this target.
include asset_core/CMakeFiles/baker.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include asset_core/CMakeFiles/baker.dir/compiler_depend.make

# Include the progress variables for this target.
include asset_core/CMakeFiles/baker.dir/progress.make

# Include the compile flags for this target's objects.
include asset_core/CMakeFiles/baker.dir/flags.make

asset_core/CMakeFiles/baker.dir/asset_main.cpp.o: asset_core/CMakeFiles/baker.dir/flags.make
asset_core/CMakeFiles/baker.dir/asset_main.cpp.o: asset_core/asset_main.cpp
asset_core/CMakeFiles/baker.dir/asset_main.cpp.o: asset_core/CMakeFiles/baker.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object asset_core/CMakeFiles/baker.dir/asset_main.cpp.o"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT asset_core/CMakeFiles/baker.dir/asset_main.cpp.o -MF CMakeFiles/baker.dir/asset_main.cpp.o.d -o CMakeFiles/baker.dir/asset_main.cpp.o -c /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core/asset_main.cpp

asset_core/CMakeFiles/baker.dir/asset_main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/baker.dir/asset_main.cpp.i"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core/asset_main.cpp > CMakeFiles/baker.dir/asset_main.cpp.i

asset_core/CMakeFiles/baker.dir/asset_main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/baker.dir/asset_main.cpp.s"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core/asset_main.cpp -o CMakeFiles/baker.dir/asset_main.cpp.s

# Object files for target baker
baker_OBJECTS = \
"CMakeFiles/baker.dir/asset_main.cpp.o"

# External object files for target baker
baker_EXTERNAL_OBJECTS =

asset_core/baker: asset_core/CMakeFiles/baker.dir/asset_main.cpp.o
asset_core/baker: asset_core/CMakeFiles/baker.dir/build.make
asset_core/baker: third_party/libtinyobjloader.a
asset_core/baker: asset_core/libjson.a
asset_core/baker: asset_core/CMakeFiles/baker.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable baker"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/baker.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
asset_core/CMakeFiles/baker.dir/build: asset_core/baker
.PHONY : asset_core/CMakeFiles/baker.dir/build

asset_core/CMakeFiles/baker.dir/clean:
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core && $(CMAKE_COMMAND) -P CMakeFiles/baker.dir/cmake_clean.cmake
.PHONY : asset_core/CMakeFiles/baker.dir/clean

asset_core/CMakeFiles/baker.dir/depend:
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/asset_core/CMakeFiles/baker.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : asset_core/CMakeFiles/baker.dir/depend

