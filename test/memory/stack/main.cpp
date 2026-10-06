extern "C" {
#include <stdio.h>
#include <stdint.h>
}

#include "descent/type/core.hpp"
#include <descent/memory/stack.hpp>
#include <descent/system.hpp>
#include <descent/type/core.hpp>

namespace descent::memory {

struct Tracker {
	static u64 constructed;
	static u64 destroyed;

	u64 value;

	Tracker(u64 value = 0) : value(value) {
		++constructed;
	}

	~Tracker() {
		++destroyed;
	}
};

static inline bool is_aligned(void *p, u64 align) {
	return (reinterpret_cast<uintptr_t>(p) % align) == 0;
}

u64 Tracker::constructed = 0;
u64 Tracker::destroyed = 0;

void test_size() {
	auto b1 = stack::Allocator::create<u8>(1);
	sys_assert(b1.size() == 1, "Incorrect box size");

	auto b2 = stack::Allocator::create<u16>(2);
	sys_assert(b2.size() == 2, "Incorrect box size");

	auto b3 = stack::Allocator::create<u32>(3);
	sys_assert(b3.size() == 3, "Incorrect box size");

	auto b4 = stack::Allocator::create<u64>(5);
	sys_assert(b4.size() == 5, "Incorrect box size");

	auto b5 = stack::Allocator::create<u128>(7);
	sys_assert(b5.size() == 7, "Incorrect box size");
}

void test_constructor() {
	constexpr u8 VALUE = 0x1A;
	auto b1 = stack::Allocator::create<u8>(8, VALUE);
	sys_assert(b1, "Incorrect box size");
	for (u64 i = 0; i  < b1.size(); ++i)
		sys_assert(b1[i] == VALUE, "Incorrectly constructed element");
}

void test_align() {
	auto b1 = stack::Allocator::create<u8>(1);
	sys_assert(is_aligned(&b1[0], alignof(u8)), "Incorrect box alignment");

	auto b2 = stack::Allocator::create<u16>(2);
	sys_assert(is_aligned(&b2[0], alignof(u16)), "Incorrect box alignment");

	auto b3 = stack::Allocator::create<u32>(3);
	sys_assert(is_aligned(&b3[0], alignof(u32)), "Incorrect box alignment");

	auto b4 = stack::Allocator::create<u64>(5);
	sys_assert(is_aligned(&b4[0], alignof(u64)), "Incorrect box alignment");

	auto b5 = stack::Allocator::create<u128>(7);
	sys_assert(is_aligned(&b5[0], alignof(u128)), "Incorrect box alignment");
}

void test_lifo_reclamation() {
	auto a = stack::Allocator::create<u64>(8);
	auto *a_ptr = a.data();

	auto b = stack::Allocator::create<u64>(8);
	auto *b_ptr = b.data();

	auto c = stack::Allocator::create<u64>(8);
	auto *c_ptr = c.data();

	auto d = stack::Allocator::create<u64>(8);
	auto *d_ptr = d.data();

	sys_assert(a, "Allocation A failed");
	sys_assert(b, "Allocation B failed");
	sys_assert(c, "Allocation C failed");
	sys_assert(d, "Allocation D failed");

	d.~Box();
	sys_assert(!d, "Allocation D was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		sys_assert(e.data() == d_ptr, "Top allocation was not reclaimed");
	}

	a.~Box();
	sys_assert(!a, "Allocation A was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		sys_assert(e.data() == d_ptr, "Allocation shifted on non-contiguous frees");
	}

	c.~Box();
	sys_assert(!c, "Allocation C was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		sys_assert(e.data() == c_ptr, "Top allocation was not reclaimed");
	}

	b.~Box();
	sys_assert(!b, "Allocation B was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		sys_assert(e.data() == a_ptr, "Contiguous frees were not reclaimed");
	}
}

void test_loop() {
	auto base = stack::Allocator::create<u8>(1);
	auto *base_ptr = base.data();
	base.~Box();

	for (u64 i = 0; i < 100; ++i) {
		auto box = stack::Allocator::create<u8>(12);
	}
	
	auto box = stack::Allocator::create<u8>(12);
	sys_assert(box.data() == base_ptr, "Frees were not reclaimed");
}

void test_nested() {
	auto outer = stack::Allocator::create<u64>(1024);

	for (u64 i = 0; i < outer.size(); ++i)
		outer[i] = i;

	{
		auto middle = stack::Allocator::create<u64>(2048);

		for (u64 i = 0; i < middle.size(); ++i)
			middle[i] = i * 3;

		{
			auto inner = stack::Allocator::create<u64>(4096);

			for (u64 i = 0; i < inner.size(); ++i)
				inner[i] = i * 7;
		}

		for (u64 i = 0; i < middle.size(); ++i)
			sys_assert(middle[i] == i * 3, "Middle allocation corrupted");
	}

	for (u64 i = 0; i < outer.size(); ++i)
		sys_assert(outer[i] == i, "Outer allocation corrupted");
}

void test_object_lifetime() {
	Tracker::constructed = 0;
	Tracker::destroyed = 0;

	{
		auto box = stack::Allocator::create<Tracker>(100, 42);

		sys_assert(
			Tracker::constructed == 100,
			"Incorrect number of constructions"
		);

		for (u64 i = 0; i < box.size(); ++i)
			sys_assert(box[i].value == 42, "Incorrect constructor argument");
	}

	sys_assert(
		Tracker::destroyed == 100,
		"Incorrect number of destructions"
	);
}

void test_reuse() {
	for (u64 iteration = 0; iteration < 100000; ++iteration) {
		auto base = stack::Allocator::create<u64>(1);
		auto base_ptr = base.data();
		base.~Box();

		{
			auto a = stack::Allocator::create<u64>(1);
			a[0] = iteration;
		}

		auto ba = stack::Allocator::create<u64>(1);
		auto ba_ptr = ba.data();
		sys_assert(ba_ptr == base_ptr, "Freed box A was not reclaimed");
		ba.~Box();

		{
			auto b = stack::Allocator::create<u64>(2);
			b[0] = iteration;
			b[1] = iteration + 1;
		}

		auto bb = stack::Allocator::create<u64>(1);
		auto bb_ptr = bb.data();
		sys_assert(bb_ptr == base_ptr, "Freed box B was not reclaimed");
		bb.~Box();

		{
			auto c = stack::Allocator::create<u64>(3);
			c[0] = iteration;
			c[1] = iteration + 1;
			c[2] = iteration + 2;
		}

		auto bc = stack::Allocator::create<u64>(1);
		auto bc_ptr = bc.data();
		sys_assert(bc_ptr == base_ptr, "Freed box C was not reclaimed");
		bc.~Box();
	}
}

void stress_varying_sizes() {
	for (u64 iteration = 0; iteration < 10000; ++iteration) {
		u64 count_a = 1 + (iteration % 97);
		u64 count_b = 1 + (iteration % 193);
		u64 count_c = 1 + (iteration % 389);

		{
			auto a = stack::Allocator::create<u64>(count_a);
			auto b = stack::Allocator::create<u64>(count_b);
			auto c = stack::Allocator::create<u64>(count_c);

			for (u64 i = 0; i < a.size(); ++i)
				a[i] = i ^ iteration;

			for (u64 i = 0; i < b.size(); ++i)
				b[i] = i ^ (iteration << 1);

			for (u64 i = 0; i < c.size(); ++i)
				c[i] = i ^ (iteration << 2);

			for (u64 i = 0; i < a.size(); ++i)
				sys_assert(a[i] == (i ^ iteration), "A corrupted");

			for (u64 i = 0; i < b.size(); ++i)
				sys_assert(b[i] == (i ^ (iteration << 1)), "B corrupted");

			for (u64 i = 0; i < c.size(); ++i)
				sys_assert(c[i] == (i ^ (iteration << 2)), "C corrupted");
		}
	}
}

}

using namespace descent;
using namespace descent::memory;

struct Test {
	const char* name;
	void (*function)();
};

static constexpr Test tests[] = {
	{ "test_size",                 test_size },
	{ "test_constructor",          test_constructor },
	{ "test_align",                test_align },
	{ "test_lifo_reclamation",     test_lifo_reclamation },
	{ "test_loop",                 test_loop },
	{ "test_nested",               test_nested },
	{ "test_object_lifetime",      test_object_lifetime },
	{ "test_reuse",                test_reuse },
	{ "stress_varying_sizes",      stress_varying_sizes }
};

int main() {
	for (u64 i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		fprintf(stderr, "[TEST] %s\n", tests[i].name);
		tests[i].function();
		fprintf(stderr, "[PASS] %s\n", tests[i].name);
	}

	return 0;
}
