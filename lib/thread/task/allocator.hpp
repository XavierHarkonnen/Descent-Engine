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

#ifndef DESCENT_LIB_THREAD_TASK_ALLOCATOR_HPP
#define DESCENT_LIB_THREAD_TASK_ALLOCATOR_HPP

#include <descent/type.hpp>
#include <descent/memory/region.hpp>

#include "node.hpp"

namespace descent::thread::task {

class Allocator {
private:
	static constexpr u64 BLOCK_CAPACITY = 1024;
	static constexpr u64 LOCAL_CAPACITY = BLOCK_CAPACITY - 1;
	static constexpr u64 BLOCK_SIZE = BLOCK_CAPACITY * sizeof(Task);

	static_assert(BLOCK_SIZE % memory::PAGE_SIZE == 0, "BLOCK_SIZE must be a multiple of PAGE_SIZE");

	static constexpr u64 BACKUP_COUNT = 6;
	static constexpr u32 BLOCK_PAGES = BLOCK_SIZE / memory::PAGE_SIZE;

	Task storage[LOCAL_CAPACITY];
	memory::Region<Task, BLOCK_PAGES> backup[BACKUP_COUNT];
	u64 size;
	u64 cursor;

public:
	Allocator() : size(LOCAL_CAPACITY), cursor(0) {}

	Task *allocate() {
		if (cursor < size)
			return &storage[cursor++];

		const u64 adjusted_cursor = cursor - LOCAL_CAPACITY;
		const u64 backup_index = adjusted_cursor / BLOCK_CAPACITY;
		const u64 backup_cursor = adjusted_cursor % BLOCK_CAPACITY;

		if (backup_index >= BACKUP_COUNT)
			return nullptr;

		if (!backup[backup_index])
			if (!backup[backup_index].acquire())
				return nullptr;

		++cursor;

		return &backup[backup_index][backup_cursor];
	}

	void clear() {
		cursor = 0;
	}
};

}

#endif
