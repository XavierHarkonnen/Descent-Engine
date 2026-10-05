#include <descent/memory/stack.hpp>

extern "C" {
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
}

#include <descent/memory.hpp>
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
	void init(u32 size) {
		sys_assert(size != 0, "Footer size must not be zero");
		sys_assert((size & DEAD_MASK) == 0, "Size must not be set live bit");
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

class Allocator {
private:
	static constexpr u32 CAPACITY  = 0x80000000u;
	static constexpr u32 WIPE_SIZE = 0x100000u;

	u8 *_map;
	u32 _size;
	u32 _cursor;

public:
	Allocator() : _map(nullptr), _size(0), _cursor(0) {
		void *map = mmap(nullptr, CAPACITY, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (map != MAP_FAILED) {
			_map = static_cast<u8 *>(map);
			_size = CAPACITY;
		}
	}

	~Allocator() {
		if (_map) {
			munmap(_map, _size);
			_map = nullptr;
			_size = 0;
			_cursor = 0;
		}
	}

	u8 *alloc(u32 size) {
		u64 total = u64(size) + sizeof(Footer);

		if (_size - _cursor < total)
			return nullptr;

		u8 *data = _map + _cursor;
		Footer *footer = reinterpret_cast<Footer *>(data + size);
		
		_cursor += u32(total);

		footer->init(u32(total));

		return data;
	}

	void free(Footer *footer) {
		sys_assert(footer, "Attempted to free null footer");
		sys_assert(!footer->dead(), "Attempted to free dead footer");
		sys_assert(_cursor, "Attempted to free footer from empty allocator");
		sys_assert(_cursor > sizeof(Footer), "Something very bad happened");

		u8 *ptr = reinterpret_cast<u8 *>(footer);

		sys_assert(ptr >= _map + sizeof(Footer), "Footer is outside allocator");
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

static thread_local Allocator bump;

Alloc::Alloc(u32 size) : _data(nullptr), _meta(nullptr) {
	if (size != 0) {
		_data = bump.alloc(size);
		if (_data)
			_meta = _data + size;
	}
}

Alloc::~Alloc() {
	if (_data) {
		bump.free(reinterpret_cast<Footer *>(_meta));
		_data = nullptr;
		_meta = nullptr;
	}
}

void *Alloc::data() {
	return _data;
}

u32 Alloc::size() {
	return _data ? reinterpret_cast<Footer *>(_meta)->size() - sizeof(Footer) : 0;
}

}
