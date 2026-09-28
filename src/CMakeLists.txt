set(EXECUTABLE_NAME "granbarrow")

add_executable(${EXECUTABLE_NAME}
	main.cpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
	descent-memory
	descent-system
	descent-thread
	descent-time
)

target_compile_options(${EXECUTABLE_NAME} PRIVATE
)
