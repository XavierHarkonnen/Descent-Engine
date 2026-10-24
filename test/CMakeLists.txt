# The same check as the check-sources target.
descent_script_test(sources
	SCRIPT ${CMAKE_SOURCE_DIR}/cmake/CheckSources.cmake
)

# The same check as the check-format target, when clang-format 21 is available.
if(DESCENT_CLANG_FORMAT_FOUND)
	descent_script_test(format
		SCRIPT ${CMAKE_SOURCE_DIR}/cmake/CheckFormat.cmake
		VARIABLES CLANG_FORMAT=${DESCENT_CLANG_FORMAT}
	)
endif()

add_subdirectory(harness)
add_subdirectory(headers)
add_subdirectory(memory)
add_subdirectory(rengine)
add_subdirectory(thread)
add_subdirectory(type)
