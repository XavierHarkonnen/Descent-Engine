// Misuse of memory::stack::Box that must not compile. Each CASE is one misuse;
// CMakeLists.txt registers one test per case, which passes only if compiling
// this file with that CASE fails with the diagnostic the misuse should cause,
// so an unrelated compile error does not count. Without a CASE, the file is
// valid use and compiles.

#include <descent/memory/stack.hpp>

#ifndef CASE
#define CASE 0
#endif

using namespace descent;
using namespace descent::memory;

using Box = stack::Box<int>;

alignas(Box) static unsigned char storage[sizeof(Box)];

#if CASE == 5
static Box named() {
	Box box = stack::Allocator::create<int>(1);
	return box;
}
#endif

static Box prvalue() {
	return stack::Allocator::create<int>(1);
}

int main() {
	Box box = prvalue();
	(void) storage;

#if CASE == 1
	Box *p = new Box(stack::Allocator::create<int>(1));
#elif CASE == 2
	new (storage) Box(stack::Allocator::create<int>(1));
#elif CASE == 3
	Box *p = new Box[1];
#elif CASE == 4
	Box *p = reinterpret_cast<Box *>(storage);
	delete p;
#elif CASE == 5
	Box other = named();
#elif CASE == 6
	Box other = box;
#elif CASE == 7
	box = stack::Allocator::create<int>(1);
#elif CASE == 8
	memory::construct(reinterpret_cast<Box *>(storage), stack::Allocator::create<int>(1));
#endif

	return box ? 0 : 1;
}
