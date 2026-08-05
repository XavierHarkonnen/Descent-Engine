set(LIBRARY_NAME "descent-system")

add_library(${LIBRARY_NAME}
	system.c
)

target_include_directories(${LIBRARY_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/intern
)

target_link_libraries(${LIBRARY_NAME} PUBLIC
)

target_compile_options(${LIBRARY_NAME} PRIVATE
	${INTERN_C_FLAGS}
)
