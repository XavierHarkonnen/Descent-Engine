descent_library(thread
	SOURCES futex.cpp task.cpp thread.cpp utilities.cpp
	LINK descent::system descent::memory
)
