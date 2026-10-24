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

extern "C" {
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
}

#include <descent/system.hpp>
#include <descent/type/core.hpp>

namespace descent::memory::stack {

class Footer {
private:
	static constexpr u32 DEAD_MASK = 0x80000000u;
	static constexpr u32 SIZE_MASK = 0x7FFFFFFFu;

	// Using u8 alignment allows us to place a footer anywhere in the allocator's memory
	u8 _data[sizeof(u32)];

	u32 word() const {
		u32 value;
		memcpy(&value, _data, sizeof(u32));
		return value;
	}

	void word(u32 value) {
		memcpy(_data, &value, sizeof(u32));
	}

public:
	static constexpr u32 SIZE_MAX = SIZE_MASK;

	void init(u32 size) {
		sys_assert(size != 0, "Footer size must not be zero");
		sys_assert(size <= SIZE_MAX, "Size must not exceed SIZE_MAX");
		word(size);
	}

	void kill() {
		word(word() | DEAD_MASK);
	}

	bool dead() const {
		return word() & DEAD_MASK;
	}

	// Returns size of payload plus size of footer
	u32 size() const {
		return word() & SIZE_MASK;
	}
};

static_assert(sizeof(Footer) == sizeof(u32), "Architecture requires size of Footer to equal size of u32");

class Bump {
private:
	static constexpr u32 CAPACITY  = 0x80000000u;

	u8 *_map;
	u32 _size;
	u32 _cursor;

public:
	Bump() : _map(nullptr), _size(0), _cursor(0) {
		void *map = mmap(nullptr, CAPACITY, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (map != MAP_FAILED) {
			_map = static_cast<u8 *>(map);
			_size = CAPACITY;
		}
	}

	~Bump() {
		// The main thread is exempt, since exit() destroys its thread_local
		// storage while Boxes may still be alive in its frames
		sys_assert(_cursor == 0 || gettid() == getpid(), "Thread exited while a Box it allocated was alive");

		if (_map) {
			munmap(_map, _size);
			_map = nullptr;
			_size = 0;
			_cursor = 0;
		}
	}

	bool alloc(void *&data, void *&meta, u32 size, u32 align, u32 count) {
		sys_assert(size != 0, "Attempted to allocate box with zero size");
		sys_assert(align != 0, "Attempted to allocate box with zero alignment");
		sys_assert(count != 0, "Attempted to allocate box with zero count");
		sys_assert((align & (align - 1)) == 0, "Alignment must be a power of two");

		u64 payload = u64(size) * u64(count);
		u64 aligned = (u64(_cursor) + (u64(align) - 1)) & ~(u64(align) - 1);
		u64 padding = aligned - _cursor;

		// This multiplication is proven to never overflow
		u64 total = payload + padding + sizeof(Footer);
		if (_size - _cursor < total || total > Footer::SIZE_MAX) {
			data = nullptr;
			meta = nullptr;
			return false;
		}

		u8 *alloc = _map + aligned;
		Footer *footer = reinterpret_cast<Footer *>(alloc + payload);
		data = alloc;
		meta = footer;

		_cursor += u32(total);

		footer->init(u32(total));

		sys_assert(_cursor <= _size, "Allocator cursor is outside of allocation range");
		sys_assert(_cursor > sizeof(Footer), "Allocator cursor is misaligned");

		return true;
	}

	bool owns(const void *ptr) const {
		const u8 *byte = static_cast<const u8 *>(ptr);
		return byte >= _map && byte < _map + _size;
	}

	void free(Footer *footer) {
		sys_assert(footer, "Attempted to free null footer");
		sys_assert(owns(footer), "Box was freed on a thread other than the one that allocated it");
		sys_assert(!footer->dead(), "Attempted to free dead footer");
		sys_assert(_cursor > 0, "Attempted to free footer from empty allocator");
		sys_assert(footer->size() > sizeof(Footer), "Attempted to free invalid footer");

		sys_assert(_cursor <= _size, "Allocator cursor is outside of allocation range");
		sys_assert(_cursor > sizeof(Footer), "Allocator cursor is misaligned");

		u8 *ptr = reinterpret_cast<u8 *>(footer);

		sys_assert(ptr >= _map, "Footer is outside allocated range");
		sys_assert(ptr >= _map + 1, "Footer is not attached to allocation (minimum size 1 byte)");
		sys_assert(ptr + sizeof(Footer) <= _map + _cursor, "Footer is outside allocated range");

		// This allocation isn't the top allocation
		if (ptr + sizeof(Footer) != _map + _cursor) {
			footer->kill();
			return;
		}

		// This allocation is the top allocation
		// Reclaim it and all connected dead allocations
		for (;;) {
			u32 total = footer->size();
			sys_assert(total >= sizeof(Footer), "Invalid footer size");
			sys_assert(total <= _cursor, "Dead footer extends outside allocator");

			_cursor -= total;

			if (_cursor == 0)
				break;

			footer = reinterpret_cast<Footer *>(_map + _cursor - sizeof(Footer));
			if (!footer->dead())
				break;
		}
	}
};

static thread_local Bump bump;

#if defined(DESCENT_DEBUG)
// Bounds of the calling thread's stack, where every live Box must lie
class Range {
private:
	const u8 *_low;
	const u8 *_high;

public:
	Range() : _low(nullptr), _high(nullptr) {
		pthread_attr_t attr;
		if (pthread_getattr_np(pthread_self(), &attr) != 0)
			return;

		void *low;
		__SIZE_TYPE__ size;
		if (pthread_attr_getstack(&attr, &low, &size) == 0) {
			_low = static_cast<const u8 *>(low);
			_high = _low + size;
		}

		pthread_attr_destroy(&attr);
	}

	// Accepts everything if the bounds are unknown
	bool contains(const void *ptr) const {
		const u8 *byte = static_cast<const u8 *>(ptr);
		return !_low || (byte >= _low && byte < _high);
	}
};

static thread_local Range range;
#endif

bool Allocator::create(void *&data, void *&meta, u32 size, u32 align, u32 capacity) {
	return bump.alloc(data, meta, size, align, capacity);
}

#if defined(DESCENT_DEBUG)
void Allocator::local(const void *box) {
	sys_assert(range.contains(box), "Box is not a local variable of the calling thread "
		"(it is static, thread_local, on the heap or on another thread's stack)");
}
#endif

void Allocator::clear([[maybe_unused]] void *data, void *meta) {
	sys_assert(data, "Attempted to call clear on null data");
	sys_assert(meta, "Attempted to call clear on null meta");

	bump.free(reinterpret_cast<Footer *>(meta));
}

}
