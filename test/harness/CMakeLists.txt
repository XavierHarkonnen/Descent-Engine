# The shared test harness. descent_test links it into every test.
add_library(descent-test INTERFACE)
add_library(descent::test ALIAS descent-test)

target_include_directories(descent-test INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)
