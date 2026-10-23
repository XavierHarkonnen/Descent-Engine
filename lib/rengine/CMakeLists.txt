set(LIBRARY_NAME "descent-rengine")

add_library(${LIBRARY_NAME}
	rengine.cpp
)

target_include_directories(${LIBRARY_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/intern
)

target_link_libraries(${LIBRARY_NAME} PUBLIC
	PkgConfig::WAYLAND
)

target_compile_options(${LIBRARY_NAME} PRIVATE
	${INTERN_CXX_FLAGS}
)
