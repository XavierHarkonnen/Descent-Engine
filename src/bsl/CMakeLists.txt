descent_executable(granbarrow-bsl
	SOURCES main.cpp
	LINK
		descent::file
		descent::memory
		descent::rengine
		descent::system
		descent::thread
		descent::time
)
