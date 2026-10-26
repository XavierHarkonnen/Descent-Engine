descent_executable(granbarrow-server
	SOURCES main.cpp
	LINK
		descent::file
		descent::memory
		descent::system
		descent::thread
		descent::time
)
