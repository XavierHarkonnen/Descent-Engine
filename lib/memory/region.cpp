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

#include <descent/memory/region.hpp>

#include <sys/mman.h>

#include <descent/memory/platform.hpp>
#include <descent/type/core.hpp>

namespace descent::memory {

static inline int mem_prot(Access access) {
	switch (access) {
		case Access::NONE: return PROT_NONE;
		case Access::READ: return PROT_READ;
		case Access::EDIT: return PROT_READ | PROT_WRITE;
	}
	__builtin_unreachable();
}

void *Detail::acquire(u32 page_count, Access access) {
	u64 size = PAGE_SIZE * page_count;

	void *data = mmap(nullptr, size, mem_prot(access), MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (data == MAP_FAILED)
		data = nullptr;

	return data;
}

void *Detail::resize(void *data, u32 page_count, u32 new_count) {
	u64 size = PAGE_SIZE * page_count;
	u64 new_size = PAGE_SIZE * new_count;

	void *new_data = mremap(data, size, new_size, MREMAP_MAYMOVE);
	if (new_data == MAP_FAILED)
		new_data = nullptr;

	return new_data;
}

bool Detail::protect(void *data, u32 page_start, u32 page_count, Access access) {
	u64 start = PAGE_SIZE * page_start;
	u64 size = PAGE_SIZE * page_count;

	if (mprotect(static_cast<u8 *>(data) + start, size, mem_prot(access)))
		return false;

	return true;
}

void Detail::release(void *data, u32 page_count) {
	munmap(data, PAGE_SIZE * page_count);
}

}
