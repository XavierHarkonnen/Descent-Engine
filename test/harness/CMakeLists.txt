# The shared test harness. descent_test links it into every test.
descent_library(test HEADER_ONLY
	INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}
)
