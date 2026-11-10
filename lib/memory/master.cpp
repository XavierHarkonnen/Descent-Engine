/* Copyright 2025 XavierHarkonnen9 and Enlarium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
}

#include <descent/type/bit.hpp>
#include <descent/type/traits.hpp>
#include <descent/system.hpp>

#ifndef MAP_FIXED_NOREPLACE
#error "MAP_FIXED_NOREPLACE is not defined"
#endif

// TODO: replace strtoull with strto64

namespace descent::memory {

// Interface

enum class Access {
	NONE,
	READ,
	EDIT
};

enum class Class : u64 {
	C24 = 24,
	C32 = 32,
	C40 = 40
};

template <typename T, typename B, u8 rank>
class Tag {
private:
	static_assert(rank != 0, "Rank 0 is invalid");
	static_assert(rank <= 64, "Rank greater than 64 is invalid");
	static_assert(type::traits<B>::is_unsigned, "Base type must be an unsigned integer");

	static constexpr u64 BITS_BASE = type::traits<B>::bits;
	static_assert(BITS_BASE <= 64, "Base type must be 64 or fewer bits wide");

	static constexpr u64 BITS_LPTR = bit::ctz(alignof(T));
	static constexpr u64 BITS_HPTR = rank;
	static_assert(BITS_HPTR > BITS_LPTR, "Rank is insufficient for alignment of type");

	static constexpr u64 BITS_PTR = BITS_HPTR - BITS_LPTR;
	static_assert(BITS_BASE >= BITS_PTR, "Base has insufficient width for type");

	static constexpr u64 BITS_TAG = BITS_BASE - BITS_PTR;
	static constexpr u64 MASK_PTR = ((u64(1) << BITS_PTR) - 1) << BITS_TAG;
	static constexpr u64 MASK_TAG = (u64(1) << BITS_TAG) - 1;

	static constexpr u64 BITS_SHIFT = BITS_BASE - BITS_HPTR;

	B value;

public:
	constexpr u64 tag_bits() {
		return BITS_TAG;
	}

	constexpr B tag() {
		return value & MASK_TAG;
	}

	constexpr void tag(B tag) {
		sysd_assert((tag & MASK_PTR) == 0, "Tag has illegal bits set");
		value &= MASK_PTR;
		value |= (tag & MASK_TAG);
	}

	constexpr bool get(u8 index) {
		if (index >= BITS_TAG)
			sys_fatal("Illegal bit index");
		
		return value & B(1) << index;
	}

	void set(u8 index) {
		if (index >= BITS_TAG)
			sys_fatal("Illegal bit index");
		
		value |= B(1) << index;
	}

	bool clear(u8 index) {
		if (index >= BITS_TAG)
			sys_fatal("Illegal bit index");
		
		value &= ~(B(1) << index);
	}

	constexpr u64 ptr_bits() {
		return BITS_PTR;
	}

	T &operator *() {
		return *reinterpret_cast<T *>((value & MASK_PTR) >> BITS_SHIFT);
	}
};

// TODO: print method for testing

// Implementation

template <Class C>
struct Arena {
	void *base;
	u64 size;

	Arena(void *base, u64 size) : base(base), size(size) {}
};

inline bool u64_from_string(u64 &out, const char *string) {
	if (!string || !*string)
		return false;

	u64 result = 0;

	for (const char *p = string; *p; ++p) {
		const u8 digit = u8(*p - '0');

		if (digit > 9)
			return false;

		if (result > (U64_MAX - digit) / 10)
			return false;

		result = result * 10 + digit;
	}

	out = result;
	return true;
}

class Lmap {
	Lmap(Lmap&&) = delete;
	Lmap(const Lmap&) = delete;
	Lmap& operator=(Lmap&&) = delete;
	Lmap& operator=(const Lmap&) = delete;

private:
	static u64 init_page_size() {
		const long result = sysconf(_SC_PAGESIZE);
		if (result <= 0)
			sys_fatal("Failed to get page size");

		const u64 page_size = u64(result);
		if (bit::popcnt(page_size) != 1)
			sys_fatal("Page size is not a power of two");

		return u64(page_size);
	}

	static u64 init_min(const u64 page_size) {
		char buf[32];

		int fd = open("/proc/sys/vm/mmap_min_addr", O_RDONLY);
		if (fd < 0)
			sys_fatal("vm.mmap_min_addr cannot be opened");

		ssize_t len;
		do {
			len = read(fd, buf, sizeof(buf) - 1);
		} while (len < 0 && errno == EINTR);

		close(fd);

		while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ' || buf[len - 1] == '\t'))
			--len;

		if (len <= 0)
			sys_fatal("vm.mmap_min_addr cannot be read");

		buf[len] = '\0';

		u64 min = 0;
		if (!u64_from_string(min, buf))
			sys_fatal("vm.mmap_min_addr cannot be parsed");

		if (min == 0)
			return page_size;

		if (min > U64_MAX - (page_size - 1))
			sys_fatal("vm.mmap_min_addr alignment overflows");

		min = (min + page_size - 1) & ~(page_size - 1);

		if (min >= AR24_MAX)
			sys_fatal("No usable 24-bit address range");

		return min;
	}

	static u64 init_size(const u64 lmap_min) {
		return AR48_MAX - lmap_min;
	}

	static u8 *init_lmap(const u64 lmap_min, const u64 lmap_size) {
		void *hint = reinterpret_cast<void *>(lmap_min);
		void *map = mmap(hint, lmap_size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE, -1, 0);
		if (map == MAP_FAILED)
			sys_fatal("Failed to reserve mapping for lmap");

		if (map != hint) {
			munmap(map, lmap_size);
			sys_fatal("Kernel supplied unacceptable mapping for lmap");
		}

		return reinterpret_cast<u8 *>(map);
	}

	static constexpr u64 AR24_MAX = u64(1) << 24; // 16 MiB
	static constexpr u64 AR32_MAX = u64(1) << 32; // 4 GiB
	static constexpr u64 AR40_MAX = u64(1) << 40; // 1 TiB
	
	u64 const PAGE_SIZE = init_page_size();
	u64 const LMAP_MIN  = init_min(PAGE_SIZE);
	u64 const LMAP_SIZE = init_size(LMAP_MIN);
	u8 *const LMAP      = init_lmap(LMAP_MIN, LMAP_SIZE);

	const Arena<Class::C24> _arena_24;
	const Arena<Class::C32> _arena_32;
	const Arena<Class::C40> _arena_40;

	Lmap() :
		_arena_24(reinterpret_cast<u8 *>(LMAP_MIN), AR24_MAX - LMAP_MIN),
		_arena_32(reinterpret_cast<u8 *>(AR24_MAX), AR32_MAX - AR24_MAX),
		_arena_40(reinterpret_cast<u8 *>(AR32_MAX), AR40_MAX - AR32_MAX) {}

public:
	static void init() {
		(void) singleton();
	}

	static Lmap& singleton() {
		static Lmap Lmap;
		return Lmap;
	}

	Arena<Class::C24> arena_24() {
		return _arena_24;
	}
	
	Arena<Class::C32> arena_32() {
		return _arena_32;
	}
	
	Arena<Class::C40> arena_40() {
		return _arena_40;
	}
};


}
