# Target helpers. Every first-party library, executable and test is defined
# through these, so per-target conventions live in one place.
#
# Libraries are referenced only by their alias, descent::<module>. The real
# target, descent-<module>, is named here and nowhere else.

# descent_library(<module> SOURCES <file>... [LINK <target>...])
#
# Defines descent-<module> and its alias descent::<module>. LINK targets are
# linked PUBLIC, in the order given.
function(descent_library MODULE)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;LINK")
	set(TARGET descent-${MODULE})

	add_library(${TARGET}
		${ARG_SOURCES}
	)
	add_library(descent::${MODULE} ALIAS ${TARGET})

	target_include_directories(${TARGET} PUBLIC
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/intern
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
# Defines the executable test-descent-<name> and registers it with CTest under
# the same name.
function(descent_test NAME)
	cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;LINK")
	set(TARGET test-descent-${NAME})

	descent_executable(${TARGET}
		SOURCES ${ARG_SOURCES}
		LINK ${ARG_LINK}
	)

	add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction()
