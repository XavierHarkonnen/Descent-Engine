# The same check as the check-sources target.
add_test(NAME test-descent-sources COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/CheckSources.cmake)

add_subdirectory(harness)
add_subdirectory(headers)
add_subdirectory(memory)
add_subdirectory(rengine)
add_subdirectory(thread)
add_subdirectory(type)
