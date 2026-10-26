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

#include <descent/memory/stack.hpp>
#include <descent/type/core.hpp>

#include <descent/thread/task.hpp>

#include "stdio.h"

using namespace descent;

void routine(thread::task::Handle, void *) {
	auto box = memory::stack::Allocator::create<u64>(1024);

	printf("Box Size: %lu\n", box.size());
}

int main() {
	printf("Worker Thread Count: %lu\n", thread::task::get());

	thread::task::set(8);

	thread::task::Job jobs[] = {
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr },
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr },
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr },
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr },
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr },
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr }, { routine, nullptr },
		{ routine, nullptr }, { routine, nullptr }, { routine, nullptr },
	};

	printf("Worker Thread Count: %lu\n", thread::task::get());

	thread::task::begin(jobs);

	thread::task::end();

	thread::task::set(0);

	printf("Worker Thread Count: %lu\n", thread::task::get());

	return 0;
}
