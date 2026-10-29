cmake_minimum_required(VERSION 3.26)

project(descent-engine LANGUAGES CXX)

# Reject unsupported compilers before anything is built
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(FATAL_ERROR "Descent Engine requires Clang, but the C++ compiler is ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER}). Use a preset, or set CMAKE_CXX_COMPILER=clang++.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DESCENT_BUILD_TESTS "Build Descent test programs" OFF)
option(DESCENT_BUILD_CLIENT "Build the client and the renderer it needs (Vulkan and Wayland)" ON)

# Require out-of-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source build not allowed!")
endif()

# Include modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(DescentTargets)

# Source hygiene
add_custom_target(check-sources
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/CheckSources.cmake
	VERBATIM
)

# Formatting against .clang-format
find_program(DESCENT_CLANG_FORMAT NAMES clang-format-21 clang-format DOC "clang-format 21, for check-format")
set(DESCENT_CLANG_FORMAT_FOUND FALSE)
if(DESCENT_CLANG_FORMAT)
	execute_process(COMMAND ${DESCENT_CLANG_FORMAT} --version OUTPUT_VARIABLE DESCENT_CLANG_FORMAT_VERSION ERROR_QUIET)
	if(DESCENT_CLANG_FORMAT_VERSION MATCHES "clang-format version 21\\.")
		set(DESCENT_CLANG_FORMAT_FOUND TRUE)
	endif()
endif()

if(DESCENT_CLANG_FORMAT_FOUND)
	add_custom_target(check-format
		COMMAND ${CMAKE_COMMAND} -DCLANG_FORMAT=${DESCENT_CLANG_FORMAT} -P ${CMAKE_SOURCE_DIR}/cmake/CheckFormat.cmake
		VERBATIM
	)
else()
	message(STATUS "clang-format 21 not found: check-format is unavailable")
	add_custom_target(check-format
		COMMAND ${CMAKE_COMMAND} -E echo "check-format: clang-format 21 not found; set DESCENT_CLANG_FORMAT"
		COMMAND ${CMAKE_COMMAND} -E false
		VERBATIM
	)
endif()

# Flags
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Separate internal C flags
separate_arguments(INTERN_CXX_FLAGS)

# The engine does not use exceptions or RTTI
add_compile_options(
	-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=
	$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
	$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

# Program subdirectories
add_subdirectory(lib)
add_subdirectory(src)

# Test subdirectory
if(DESCENT_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

# Scratch subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scratch/CMakeLists.txt")
	add_subdirectory(scratch)
endif()
