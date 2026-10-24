# Format check, run as a script:
#
#   cmake -DCLANG_FORMAT=<clang-format 21> -P cmake/CheckFormat.cmake
#
# Reports every first-party source (see DescentSources.cmake) that
# .clang-format would change, without rewriting it, and fails if there is any.
# To fix them, run clang-format -i on the files it names.

cmake_minimum_required(VERSION 3.26)

get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT CLANG_FORMAT)
	message(FATAL_ERROR "check-format: pass -DCLANG_FORMAT=<path to clang-format 21>")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/DescentSources.cmake")
list(LENGTH DESCENT_SOURCES COUNT)

execute_process(
	COMMAND ${CLANG_FORMAT} --dry-run --Werror --style=file ${DESCENT_SOURCES}
	WORKING_DIRECTORY "${ROOT}"
	RESULT_VARIABLE RESULT
)

if(NOT RESULT EQUAL 0)
	message(FATAL_ERROR "check-format: the files above need formatting (clang-format -i <file>)")
endif()
message(STATUS "check-format: ${COUNT} files formatted")
