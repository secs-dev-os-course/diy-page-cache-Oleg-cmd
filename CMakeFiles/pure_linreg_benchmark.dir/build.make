# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/diy-page-cache-Oleg-cmd

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/diy-page-cache-Oleg-cmd

# Include any dependencies generated for this target.
include CMakeFiles/pure_linreg_benchmark.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/pure_linreg_benchmark.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/pure_linreg_benchmark.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pure_linreg_benchmark.dir/flags.make

CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o: CMakeFiles/pure_linreg_benchmark.dir/flags.make
CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o: src/benchmarks/pure_linreg_benchmark.cpp
CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o: CMakeFiles/pure_linreg_benchmark.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ubuntu/diy-page-cache-Oleg-cmd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o"
	$(CMAKE_COMMAND) -E __run_co_compile --tidy="/usr/bin/clang-tidy;-checks=*,-android*,-cert*,-cppcoreguidelines*,-fuchsia*,-google*,-hicpp*,-llvm*,-objc*,-readability*,-performance-inefficient-vector-operation;-header-filter=include/diy_cache/*;-p=/home/ubuntu/diy-page-cache-Oleg-cmd;--extra-arg-before=--driver-mode=g++" --source=/home/ubuntu/diy-page-cache-Oleg-cmd/src/benchmarks/pure_linreg_benchmark.cpp -- /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o -MF CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o.d -o CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o -c /home/ubuntu/diy-page-cache-Oleg-cmd/src/benchmarks/pure_linreg_benchmark.cpp

CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/diy-page-cache-Oleg-cmd/src/benchmarks/pure_linreg_benchmark.cpp > CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.i

CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/diy-page-cache-Oleg-cmd/src/benchmarks/pure_linreg_benchmark.cpp -o CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.s

# Object files for target pure_linreg_benchmark
pure_linreg_benchmark_OBJECTS = \
"CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o"

# External object files for target pure_linreg_benchmark
pure_linreg_benchmark_EXTERNAL_OBJECTS =

pure_linreg_benchmark: CMakeFiles/pure_linreg_benchmark.dir/src/benchmarks/pure_linreg_benchmark.cpp.o
pure_linreg_benchmark: CMakeFiles/pure_linreg_benchmark.dir/build.make
pure_linreg_benchmark: CMakeFiles/pure_linreg_benchmark.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/ubuntu/diy-page-cache-Oleg-cmd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable pure_linreg_benchmark"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pure_linreg_benchmark.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pure_linreg_benchmark.dir/build: pure_linreg_benchmark
.PHONY : CMakeFiles/pure_linreg_benchmark.dir/build

CMakeFiles/pure_linreg_benchmark.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pure_linreg_benchmark.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pure_linreg_benchmark.dir/clean

CMakeFiles/pure_linreg_benchmark.dir/depend:
	cd /home/ubuntu/diy-page-cache-Oleg-cmd && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/diy-page-cache-Oleg-cmd /home/ubuntu/diy-page-cache-Oleg-cmd /home/ubuntu/diy-page-cache-Oleg-cmd /home/ubuntu/diy-page-cache-Oleg-cmd /home/ubuntu/diy-page-cache-Oleg-cmd/CMakeFiles/pure_linreg_benchmark.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/pure_linreg_benchmark.dir/depend

