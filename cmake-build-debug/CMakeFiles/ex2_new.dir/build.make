# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/AdiYehezkeli/Documents/HUJI/OS/ex2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/ex2_new.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ex2_new.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ex2_new.dir/flags.make

CMakeFiles/ex2_new.dir/main.cpp.o: CMakeFiles/ex2_new.dir/flags.make
CMakeFiles/ex2_new.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ex2_new.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ex2_new.dir/main.cpp.o -c /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/main.cpp

CMakeFiles/ex2_new.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ex2_new.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/main.cpp > CMakeFiles/ex2_new.dir/main.cpp.i

CMakeFiles/ex2_new.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ex2_new.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/main.cpp -o CMakeFiles/ex2_new.dir/main.cpp.s

CMakeFiles/ex2_new.dir/main.cpp.o.requires:

.PHONY : CMakeFiles/ex2_new.dir/main.cpp.o.requires

CMakeFiles/ex2_new.dir/main.cpp.o.provides: CMakeFiles/ex2_new.dir/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/ex2_new.dir/build.make CMakeFiles/ex2_new.dir/main.cpp.o.provides.build
.PHONY : CMakeFiles/ex2_new.dir/main.cpp.o.provides

CMakeFiles/ex2_new.dir/main.cpp.o.provides.build: CMakeFiles/ex2_new.dir/main.cpp.o


CMakeFiles/ex2_new.dir/uthreads.cpp.o: CMakeFiles/ex2_new.dir/flags.make
CMakeFiles/ex2_new.dir/uthreads.cpp.o: ../uthreads.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/ex2_new.dir/uthreads.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ex2_new.dir/uthreads.cpp.o -c /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/uthreads.cpp

CMakeFiles/ex2_new.dir/uthreads.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ex2_new.dir/uthreads.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/uthreads.cpp > CMakeFiles/ex2_new.dir/uthreads.cpp.i

CMakeFiles/ex2_new.dir/uthreads.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ex2_new.dir/uthreads.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/uthreads.cpp -o CMakeFiles/ex2_new.dir/uthreads.cpp.s

CMakeFiles/ex2_new.dir/uthreads.cpp.o.requires:

.PHONY : CMakeFiles/ex2_new.dir/uthreads.cpp.o.requires

CMakeFiles/ex2_new.dir/uthreads.cpp.o.provides: CMakeFiles/ex2_new.dir/uthreads.cpp.o.requires
	$(MAKE) -f CMakeFiles/ex2_new.dir/build.make CMakeFiles/ex2_new.dir/uthreads.cpp.o.provides.build
.PHONY : CMakeFiles/ex2_new.dir/uthreads.cpp.o.provides

CMakeFiles/ex2_new.dir/uthreads.cpp.o.provides.build: CMakeFiles/ex2_new.dir/uthreads.cpp.o


# Object files for target ex2_new
ex2_new_OBJECTS = \
"CMakeFiles/ex2_new.dir/main.cpp.o" \
"CMakeFiles/ex2_new.dir/uthreads.cpp.o"

# External object files for target ex2_new
ex2_new_EXTERNAL_OBJECTS =

ex2_new: CMakeFiles/ex2_new.dir/main.cpp.o
ex2_new: CMakeFiles/ex2_new.dir/uthreads.cpp.o
ex2_new: CMakeFiles/ex2_new.dir/build.make
ex2_new: CMakeFiles/ex2_new.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ex2_new"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ex2_new.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ex2_new.dir/build: ex2_new

.PHONY : CMakeFiles/ex2_new.dir/build

CMakeFiles/ex2_new.dir/requires: CMakeFiles/ex2_new.dir/main.cpp.o.requires
CMakeFiles/ex2_new.dir/requires: CMakeFiles/ex2_new.dir/uthreads.cpp.o.requires

.PHONY : CMakeFiles/ex2_new.dir/requires

CMakeFiles/ex2_new.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ex2_new.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ex2_new.dir/clean

CMakeFiles/ex2_new.dir/depend:
	cd /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/AdiYehezkeli/Documents/HUJI/OS/ex2 /Users/AdiYehezkeli/Documents/HUJI/OS/ex2 /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug /Users/AdiYehezkeli/Documents/HUJI/OS/ex2/cmake-build-debug/CMakeFiles/ex2_new.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ex2_new.dir/depend

