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
include third_party/CMakeFiles/vkbootstrap.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include third_party/CMakeFiles/vkbootstrap.dir/compiler_depend.make

# Include the progress variables for this target.
include third_party/CMakeFiles/vkbootstrap.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/CMakeFiles/vkbootstrap.dir/flags.make

third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o: third_party/CMakeFiles/vkbootstrap.dir/flags.make
third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o: third_party/vkbootstrap/VkBootstrap.cpp
third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o: third_party/CMakeFiles/vkbootstrap.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o -MF CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o.d -o CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o -c /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party/vkbootstrap/VkBootstrap.cpp

third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.i"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party/vkbootstrap/VkBootstrap.cpp > CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.i

third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.s"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party/vkbootstrap/VkBootstrap.cpp -o CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.s

# Object files for target vkbootstrap
vkbootstrap_OBJECTS = \
"CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o"

# External object files for target vkbootstrap
vkbootstrap_EXTERNAL_OBJECTS =

third_party/libvkbootstrap.a: third_party/CMakeFiles/vkbootstrap.dir/vkbootstrap/VkBootstrap.cpp.o
third_party/libvkbootstrap.a: third_party/CMakeFiles/vkbootstrap.dir/build.make
third_party/libvkbootstrap.a: third_party/CMakeFiles/vkbootstrap.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libvkbootstrap.a"
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party && $(CMAKE_COMMAND) -P CMakeFiles/vkbootstrap.dir/cmake_clean_target.cmake
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vkbootstrap.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/CMakeFiles/vkbootstrap.dir/build: third_party/libvkbootstrap.a
.PHONY : third_party/CMakeFiles/vkbootstrap.dir/build

third_party/CMakeFiles/vkbootstrap.dir/clean:
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party && $(CMAKE_COMMAND) -P CMakeFiles/vkbootstrap.dir/cmake_clean.cmake
.PHONY : third_party/CMakeFiles/vkbootstrap.dir/clean

third_party/CMakeFiles/vkbootstrap.dir/depend:
	cd /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party /Users/root-kawaii/ComputerGraphics/Vulkan/hephaestus/vulkan-guide-starting-point/third_party/CMakeFiles/vkbootstrap.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : third_party/CMakeFiles/vkbootstrap.dir/depend

