cmake_minimum_required(VERSION 3.26)

project(descent-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DESCENT_BUILD_TESTS "Build Descent test programs" OFF)

# Require out-of-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source build not allowed!")
endif()

# Include modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(DescentTargets)

# Source hygiene (licence, include guards, whitespace): cmake --build <dir> --target check-sources
add_custom_target(check-sources
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/CheckSources.cmake
	VERBATIM
)

# Formatting against .clang-format, which needs clang-format 21: cmake --build <dir> --target check-format
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
	message(STATUS "clang-format 21 not found: check-format and test-descent-format are unavailable")
	add_custom_target(check-format
		COMMAND ${CMAKE_COMMAND} -E echo "check-format: clang-format 21 not found; set DESCENT_CLANG_FORMAT"
		COMMAND ${CMAKE_COMMAND} -E false
		VERBATIM
	)
endif()

# Find required libraries
find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)

# Flags
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Separate internal C flags
separate_arguments(INTERN_CXX_FLAGS)

add_compile_options(
	-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=
)

# Program Subdirectories
add_subdirectory(lib)
add_subdirectory(src)

# Test Subdirectory
if(DESCENT_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()

# Scratch Subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scratch/CMakeLists.txt")
	add_subdirectory(scratch)
endif()
