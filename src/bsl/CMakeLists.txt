set(EXECUTABLE_NAME "granbarrow-bsl")

add_executable(${EXECUTABLE_NAME}
	main.cpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
	descent-file
	descent-memory
	descent-rengine
	descent-system
	descent-thread
	descent-time
)

target_compile_options(${EXECUTABLE_NAME} PRIVATE
)
