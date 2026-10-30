descent_executable(granbarrow-server
	SOURCES main.cpp
	LINK
		descent::fs
		descent::memory
		descent::system
		descent::thread
		descent::time
)
