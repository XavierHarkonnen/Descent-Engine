set(LIBRARY_NAME "descent-assert")

add_library(${LIBRARY_NAME}
	assert.c
)

target_include_directories(${LIBRARY_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${LIBRARY_NAME} PUBLIC
)

target_compile_options(${LIBRARY_NAME} PRIVATE
	${INTERN_C_FLAGS}
)
