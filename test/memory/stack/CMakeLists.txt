descent_test(memory-stack
	SOURCES main.cpp
	LINK descent::system descent::memory
)

descent_test(memory-stack-contract
	SOURCES contract.cpp
	LINK descent::memory
)

# Valid use in box_misuse.cpp compiles and runs.
descent_test(memory-stack-box-valid
	SOURCES box_misuse.cpp
	LINK descent::memory
)

# Each misuse in box_misuse.cpp fails to compile with its own diagnostic:
# <name> <CASE> <regex>.
set(BOX_MISUSE
	new           1 "call to deleted function 'operator new'"
	placement-new 2 "call to deleted function 'operator new'"
	array-new     3 "call to deleted function 'operator new\\[\\]'"
	delete        4 "attempt to use a deleted function"
	return-named  5 "calling a private constructor of class 'descent::memory::stack::Box<int>'"
	copy          6 "call to deleted constructor of 'Box'"
	assign        7 "'operator=' is a private member of 'descent::memory::stack::Box<int>'"
	construct     8 "no matching function for call to 'operator new'"
)

while(BOX_MISUSE)
	list(POP_FRONT BOX_MISUSE MISUSE CASE REGEX)
	descent_compile_fail_test(memory-stack-box-${MISUSE}
		SOURCES box_misuse.cpp
		DEFINES CASE=${CASE}
		MATCHES "${REGEX}"
		LINK descent::memory
	)
endwhile()
