# Target helpers. Every first-party library, executable and test is defined
# through these, so per-target conventions live in one place.
#
# Libraries are referenced only by their alias, descent::<module>. The real
# target, descent-<module>, is named here and nowhere else.

# descent_library(<module> SOURCES <file>... [INCLUDE <dir>...] [LINK <target>...])
# descent_library(<module> HEADER_ONLY [INCLUDE <dir>...] [LINK <target>...])
#
# Defines descent-<module> and its alias descent::<module>. LINK targets are
# linked PUBLIC, in the order given. INCLUDE adds include directories after
# include/, for headers outside it.
#
# A library with SOURCES also sees intern/. A HEADER_ONLY library compiles
# nothing, so it is an INTERFACE library with include/ and no intern/, and its
# INCLUDE directories and LINK targets pass to whatever links it.
function(descent_library MODULE)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "HEADER_ONLY" "" "SOURCES;INCLUDE;LINK")
	set(TARGET descent-${MODULE})

	if(ARG_HEADER_ONLY AND ARG_SOURCES)
		message(FATAL_ERROR "descent_library(${MODULE}): HEADER_ONLY takes no SOURCES")
	elseif(NOT ARG_HEADER_ONLY AND NOT ARG_SOURCES)
		message(FATAL_ERROR "descent_library(${MODULE}): give SOURCES, or HEADER_ONLY")
	endif()

	if(ARG_HEADER_ONLY)
		add_library(${TARGET} INTERFACE)
		add_library(descent::${MODULE} ALIAS ${TARGET})

		target_include_directories(${TARGET} INTERFACE
			${CMAKE_SOURCE_DIR}/include
			${ARG_INCLUDE}
		)

		target_link_libraries(${TARGET} INTERFACE
			${ARG_LINK}
		)
		return()
	endif()

	add_library(${TARGET}
		${ARG_SOURCES}
	)
	add_library(descent::${MODULE} ALIAS ${TARGET})

	target_include_directories(${TARGET} PUBLIC
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/intern
		${ARG_INCLUDE}
	)

	target_link_libraries(${TARGET} PUBLIC
		${ARG_LINK}
	)

	target_compile_options(${TARGET} PRIVATE
		${INTERN_CXX_FLAGS}
	)
endfunction()

# descent_executable(<name> SOURCES <file>... [LINK <target>...])
function(descent_executable NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;LINK")

	add_executable(${NAME}
		${ARG_SOURCES}
	)

	target_include_directories(${NAME} PRIVATE
		${CMAKE_SOURCE_DIR}/include
	)

	target_link_libraries(${NAME} PRIVATE
		${ARG_LINK}
	)
endfunction()

# descent_test(<name> SOURCES <file>... [LINK <target>...])
#
# Defines the executable test-descent-<name>, linked with the test harness
# (descent::test), and registers it with CTest under the same name.
function(descent_test NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;LINK")
	set(TARGET test-descent-${NAME})

	descent_executable(${TARGET}
		SOURCES ${ARG_SOURCES}
		LINK descent::test ${ARG_LINK}
	)

	add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction()

# descent_compile_fail_test(<name> SOURCES <file>... MATCHES <regex>
#                           [DEFINES <definition>...] [LINK <target>...])
#
# Registers test-descent-<name>, which passes only if compiling the sources
# fails with output matching <regex>, so an unrelated error does not count.
# The sources form an object library of the same name, excluded from the
# normal build and compiled like a test; the test builds it with CMake.
# Concurrent builds of one tree can corrupt ninja's logs, so these tests share
# a resource lock.
function(descent_compile_fail_test NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "MATCHES" "SOURCES;DEFINES;LINK")
	set(TARGET test-descent-${NAME})

	add_library(${TARGET} OBJECT EXCLUDE_FROM_ALL
		${ARG_SOURCES}
	)

	target_include_directories(${TARGET} PRIVATE
		${CMAKE_SOURCE_DIR}/include
	)

	target_compile_definitions(${TARGET} PRIVATE
		${ARG_DEFINES}
	)

	target_link_libraries(${TARGET} PRIVATE
		descent::test
		${ARG_LINK}
	)

	add_test(NAME ${TARGET} COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TARGET})
	set_tests_properties(${TARGET} PROPERTIES
		PASS_REGULAR_EXPRESSION "${ARG_MATCHES}"
		RESOURCE_LOCK descent-build-tree
	)
endfunction()

# descent_script_test(<name> SCRIPT <file> [VARIABLES <variable>=<value>...])
#
# Registers test-descent-<name>, which runs the CMake script <file> with
# cmake -P and passes if it succeeds. Each VARIABLES entry is passed to the
# script as -D<variable>=<value>; a value cannot contain a semicolon, since
# CMake splits it into two arguments. A relative <file> is taken from the
# current source directory.
function(descent_script_test NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "SCRIPT" "VARIABLES")
	set(TEST test-descent-${NAME})

	if(NOT ARG_SCRIPT)
		message(FATAL_ERROR "descent_script_test(${NAME}): SCRIPT is required")
	endif()

	get_filename_component(SCRIPT "${ARG_SCRIPT}" ABSOLUTE BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	list(TRANSFORM ARG_VARIABLES PREPEND -D)

	add_test(NAME ${TEST} COMMAND ${CMAKE_COMMAND} ${ARG_VARIABLES} -P ${SCRIPT})
endfunction()
