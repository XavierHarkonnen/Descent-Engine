# The first-party C++ sources that the source and format checks cover: every
# .cpp and .hpp under include/, intern/, lib/, src/ and test/, as paths
# relative to ROOT, in DESCENT_SOURCES. extern/ and scratch/ are not covered.

file(GLOB_RECURSE DESCENT_SOURCES RELATIVE "${ROOT}"
	"${ROOT}/include/*.hpp"
	"${ROOT}/intern/*.hpp"
	"${ROOT}/lib/*.cpp" "${ROOT}/lib/*.hpp"
	"${ROOT}/src/*.cpp" "${ROOT}/src/*.hpp"
	"${ROOT}/test/*.cpp" "${ROOT}/test/*.hpp"
)
list(SORT DESCENT_SOURCES)
