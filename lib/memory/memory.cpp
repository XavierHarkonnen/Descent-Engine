#include <descent/memory.hpp>

#include <sys/mman.h>

#include <descent/type.hpp>

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
