set(LIBRARY_NAME "descent-rengine")

add_library(${LIBRARY_NAME}
	context.c
	debug.c
	device.c
	input.c
	instance.c
	swapchain.c
	window.c
)

target_include_directories(${LIBRARY_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/intern
)

target_link_libraries(${LIBRARY_NAME} PUBLIC
	glfw
	Vulkan::Vulkan
	descent-thread
)

target_compile_options(${LIBRARY_NAME} PRIVATE
	${INTERN_C_FLAGS}
)