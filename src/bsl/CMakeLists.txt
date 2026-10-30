descent_executable(granbarrow-bsl
	SOURCES main.cpp
	LINK
		descent::fs
		descent::memory
		descent::system
		descent::thread
		descent::time
)
