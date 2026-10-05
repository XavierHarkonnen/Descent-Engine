set(EXECUTABLE_NAME "test-descent-memory-stack")

add_test(NAME ${EXECUTABLE_NAME} COMMAND ${EXECUTABLE_NAME})

add_executable(${EXECUTABLE_NAME}
	main.cpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
	descent-system
	descent-memory
)

target_compile_options(${EXECUTABLE_NAME} PRIVATE
)