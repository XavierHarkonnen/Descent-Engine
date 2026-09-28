#ifndef DESCENT_LIB_THREAD_TASK_ALLOCATOR_HPP
#define DESCENT_LIB_THREAD_TASK_ALLOCATOR_HPP

#include <descent/type.hpp>

#include "descent/memory.hpp"
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
