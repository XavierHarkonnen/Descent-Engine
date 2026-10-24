/* Copyright 2025 XavierHarkonnen9 and Enlarium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
