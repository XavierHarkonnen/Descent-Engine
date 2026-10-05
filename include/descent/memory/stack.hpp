#ifndef DESCENT_MEMORY_STACK_HPP
#define DESCENT_MEMORY_STACK_HPP

#include <descent/type/core.hpp>

namespace descent::memory::stack {

class Alloc {
	Alloc(Alloc&&) = delete;
	Alloc(const Alloc&) = delete;
	Alloc& operator=(Alloc&&) = delete;
	Alloc& operator=(const Alloc&) = delete;

private:
	u8 *_data;
	void *_meta;

public:
	Alloc(u32 size);

	~Alloc();

	void *data();
	
	u32 size();
};

}

#endif