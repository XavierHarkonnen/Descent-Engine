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

# Find required libraries
find_package(Vulkan REQUIRED)

# Flags
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Separate internal C flags
separate_arguments(INTERN_CXX_FLAGS)

add_compile_options(
	-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=
)

# Subdirectories
#add_subdirectory(extern)
add_subdirectory(lib)
add_subdirectory(src)

# Tests
if(DESCENT_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()