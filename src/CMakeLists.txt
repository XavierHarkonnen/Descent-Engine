set(EXECUTABLE_NAME "granbarrow")

add_executable(${EXECUTABLE_NAME}
	main.c
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
	descent-rengine
	descent-assert
	descent-random
	descent-system
	glfw
)

target_compile_options(${EXECUTABLE_NAME} PRIVATE
)
