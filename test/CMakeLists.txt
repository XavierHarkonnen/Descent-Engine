# The same check as the check-sources target.
add_test(NAME test-descent-sources COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/CheckSources.cmake)

# The same check as the check-format target, when clang-format 21 is available.
if(DESCENT_CLANG_FORMAT_FOUND)
	add_test(NAME test-descent-format
		COMMAND ${CMAKE_COMMAND} -DCLANG_FORMAT=${DESCENT_CLANG_FORMAT} -P ${CMAKE_SOURCE_DIR}/cmake/CheckFormat.cmake)
endif()

add_subdirectory(harness)
add_subdirectory(headers)
add_subdirectory(memory)
if(DESCENT_BUILD_CLIENT)
	add_subdirectory(rengine)
endif()
add_subdirectory(thread)
add_subdirectory(type)
