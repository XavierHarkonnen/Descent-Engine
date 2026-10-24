# Compiles every public header on its own, twice in one translation unit, so a
# header that relies on an earlier include or lacks an include guard breaks the
# build. One source file per header is generated in the build tree.
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
	RELATIVE ${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/include/descent/*.hpp
)

set(SOURCES main.cpp)
foreach(HEADER ${HEADERS})
	string(MAKE_C_IDENTIFIER ${HEADER} NAME)
	set(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cpp)
	file(CONFIGURE OUTPUT ${SOURCE} CONTENT "#include <${HEADER}>\n#include <${HEADER}>\n")
	list(APPEND SOURCES ${SOURCE})
endforeach()

descent_test(headers
	SOURCES ${SOURCES}
)
