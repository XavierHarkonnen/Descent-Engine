# Source hygiene check, run as a script:
#
#   cmake -P cmake/CheckSources.cmake
#
# Checks every first-party source (see DescentSources.cmake) and build file
# under include/, lib/, src/, test/ and cmake/, plus the top-level files
# (LICENSE.md only for its final newline). extern/ and scratch/ are not
# checked. Each problem is printed as <file>:<line>: <problem>, and the script
# fails if there is any. Formatting is checked separately, by
# CheckFormat.cmake.
#
# Every file:
#   - ends with a newline
#   - has no trailing whitespace
#   - is indented with tabs; a line may start with a space only as the " *" of
#     a block comment
#
# Every .cpp and .hpp file also:
#   - starts with the licence header below
#   - has no doubled semicolon (other than "for (;;)")
#   - spells attributes as [[...]] (such as [[gnu::constructor]]), not
#     __attribute__((...)), which clang-format cannot keep on its own line
#
# Every CMake file also spells commands in lower case (if(, not IF().
#
# Every .hpp file is also guarded by #ifndef/#define as its first directive and
# a bare #endif as its last line. The guard is the file's path in screaming
# snake case: from include/ for public headers (include/descent/type/core.hpp
# is DESCENT_TYPE_CORE_HPP), and from the project root, prefixed with DESCENT_,
# for every other header (lib/thread/task/node.hpp is
# DESCENT_LIB_THREAD_TASK_NODE_HPP).

cmake_minimum_required(VERSION 3.26)

get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# The year is not checked.
set(LICENCE_FIRST "^/\\* Copyright [0-9][0-9][0-9][0-9] XavierHarkonnen9 and Enlarium\n")
set(LICENCE_REST [=[ *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
]=])

include("${CMAKE_CURRENT_LIST_DIR}/DescentSources.cmake")
set(SOURCES ${DESCENT_SOURCES})

file(GLOB_RECURSE BUILD_FILES RELATIVE "${ROOT}"
	"${ROOT}/include/CMakeLists.txt"
	"${ROOT}/lib/CMakeLists.txt"
	"${ROOT}/src/CMakeLists.txt"
	"${ROOT}/test/CMakeLists.txt"
	"${ROOT}/cmake/*.cmake"
)
list(SORT BUILD_FILES)
set(FILES ${SOURCES} ${BUILD_FILES} CMakeLists.txt CMakePresets.json README.md .gitignore .git-blame-ignore-revs)

# The licence text is kept verbatim, so only its final newline is checked.
set(VERBATIM_FILES LICENSE.md)

# File contents are split into lines through a CMake list, so the characters
# that lists treat specially are swapped for control characters first.
string(ASCII 1 ESC_SEMICOLON)
string(ASCII 2 ESC_OPEN)
string(ASCII 3 ESC_CLOSE)
string(ASCII 4 ESC_BACKSLASH)

set(PROBLEMS 0)

macro(problem FILE LINE TEXT)
	message(NOTICE "${FILE}:${LINE}: ${TEXT}")
	math(EXPR PROBLEMS "${PROBLEMS} + 1")
endmacro()

foreach(FILE IN LISTS VERBATIM_FILES)
	file(READ "${ROOT}/${FILE}" CONTENT)
	if(NOT CONTENT MATCHES "\n$")
		problem("${FILE}" 1 "no newline at end of file")
	endif()
endforeach()

foreach(FILE IN LISTS FILES)
	file(READ "${ROOT}/${FILE}" CONTENT)
	if(CONTENT STREQUAL "")
		continue()
	endif()

	get_filename_component(EXT "${FILE}" LAST_EXT)
	get_filename_component(NAME "${FILE}" NAME)
	set(IS_CMAKE FALSE)
	if(EXT STREQUAL ".cmake" OR NAME STREQUAL "CMakeLists.txt")
		set(IS_CMAKE TRUE)
	endif()
	set(IS_SOURCE FALSE)
	if(EXT STREQUAL ".cpp" OR EXT STREQUAL ".hpp")
		set(IS_SOURCE TRUE)
	endif()

	if(NOT CONTENT MATCHES "\n$")
		problem("${FILE}" 1 "no newline at end of file")
	endif()

	if(IS_SOURCE)
		string(LENGTH "${LICENCE_REST}" REST_LENGTH)
		string(REGEX MATCH "${LICENCE_FIRST}" FIRST "${CONTENT}")
		set(LICENSED FALSE)
		if(FIRST)
			string(LENGTH "${FIRST}" FIRST_LENGTH)
			string(SUBSTRING "${CONTENT}" ${FIRST_LENGTH} ${REST_LENGTH} REST)
			if(REST STREQUAL LICENCE_REST)
				set(LICENSED TRUE)
			endif()
		endif()
		if(NOT LICENSED)
			problem("${FILE}" 1 "missing or altered licence header")
		endif()
	endif()

	if(EXT STREQUAL ".hpp")
		string(REGEX REPLACE "^include/" "" GUARD "${FILE}")
		if(GUARD STREQUAL FILE)
			set(GUARD "descent/${FILE}")
		endif()
		string(TOUPPER "${GUARD}" GUARD)
		string(MAKE_C_IDENTIFIER "${GUARD}" GUARD)

		string(REGEX MATCH "(^|\n)#[^\n]*\n#[^\n]*" DIRECTIVES "${CONTENT}")
		string(STRIP "${DIRECTIVES}" DIRECTIVES)
		if(NOT DIRECTIVES STREQUAL "#ifndef ${GUARD}\n#define ${GUARD}")
			string(REGEX MATCH "^[^\n]*" FOUND "${DIRECTIVES}")
			problem("${FILE}" 1 "include guard must be ${GUARD} (found \"${FOUND}\")")
		endif()
		if(NOT CONTENT MATCHES "\n#endif\n?$")
			problem("${FILE}" 1 "last line must be a bare #endif")
		endif()
	endif()

	string(REPLACE "\\" "${ESC_BACKSLASH}" LINES "${CONTENT}")
	string(REPLACE ";" "${ESC_SEMICOLON}" LINES "${LINES}")
	string(REPLACE "[" "${ESC_OPEN}" LINES "${LINES}")
	string(REPLACE "]" "${ESC_CLOSE}" LINES "${LINES}")
	string(REGEX REPLACE "\n$" "" LINES "${LINES}")
	string(REPLACE "\n" ";" LINES "${LINES}")

	set(NUMBER 0)
	foreach(LINE IN LISTS LINES)
		math(EXPR NUMBER "${NUMBER} + 1")

		if(LINE MATCHES "[ \t\r]$")
			problem("${FILE}" ${NUMBER} "trailing whitespace")
		endif()
		if(LINE MATCHES "^ " AND NOT LINE MATCHES "^ \\*( |/|$)")
			problem("${FILE}" ${NUMBER} "indented with spaces")
		endif()
		if(LINE MATCHES "^\t* +\t")
			problem("${FILE}" ${NUMBER} "space before tab in indentation")
		endif()

		if(IS_CMAKE AND LINE MATCHES "^\t*[A-Z][A-Z0-9_]*[ \t]*\\(")
			problem("${FILE}" ${NUMBER} "CMake command in upper case")
		endif()

		if(IS_SOURCE)
			string(REPLACE "(${ESC_SEMICOLON}${ESC_SEMICOLON})" "" CODE "${LINE}")
			if(CODE MATCHES "${ESC_SEMICOLON}[ \t]*${ESC_SEMICOLON}")
				problem("${FILE}" ${NUMBER} "doubled semicolon")
			endif()
			if(LINE MATCHES "__attribute__")
				problem("${FILE}" ${NUMBER} "__attribute__((...)): use [[...]]")
			endif()
		endif()
	endforeach()
endforeach()

list(LENGTH FILES COUNT)
math(EXPR COUNT "${COUNT} + 1")
if(PROBLEMS GREATER 0)
	message(FATAL_ERROR "check-sources: ${PROBLEMS} problems in ${COUNT} files")
endif()
message(STATUS "check-sources: ${COUNT} files clean")
