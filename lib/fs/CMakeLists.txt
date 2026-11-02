descent_library(fs
	SOURCES handle.cpp file/file.cpp directory/directory.cpp directory/roots.cpp
	LINK descent::system descent::thread descent::memory
)
