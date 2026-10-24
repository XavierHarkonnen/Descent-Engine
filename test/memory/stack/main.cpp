extern "C" {
#include <stdint.h>
}

#include <harness.hpp>

#include <descent/memory/stack.hpp>
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
	test_check(b1.size() == 1, "Incorrect box size");

	auto b2 = stack::Allocator::create<u16>(2);
	test_check(b2.size() == 2, "Incorrect box size");

	auto b3 = stack::Allocator::create<u32>(3);
	test_check(b3.size() == 3, "Incorrect box size");

	auto b4 = stack::Allocator::create<u64>(5);
	test_check(b4.size() == 5, "Incorrect box size");

	auto b5 = stack::Allocator::create<u128>(7);
	test_check(b5.size() == 7, "Incorrect box size");
}

void test_constructor() {
	constexpr u8 VALUE = 0x1A;
	auto b1 = stack::Allocator::create<u8>(8, VALUE);
	test_check(b1, "Incorrect box size");
	for (u64 i = 0; i  < b1.size(); ++i)
		test_check(b1[i] == VALUE, "Incorrectly constructed element");
}

void test_align() {
	auto b1 = stack::Allocator::create<u8>(1);
	test_check(is_aligned(&b1[0], alignof(u8)), "Incorrect box alignment");

	auto b2 = stack::Allocator::create<u16>(2);
	test_check(is_aligned(&b2[0], alignof(u16)), "Incorrect box alignment");

	auto b3 = stack::Allocator::create<u32>(3);
	test_check(is_aligned(&b3[0], alignof(u32)), "Incorrect box alignment");

	auto b4 = stack::Allocator::create<u64>(5);
	test_check(is_aligned(&b4[0], alignof(u64)), "Incorrect box alignment");

	auto b5 = stack::Allocator::create<u128>(7);
	test_check(is_aligned(&b5[0], alignof(u128)), "Incorrect box alignment");
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

	test_check(a, "Allocation A failed");
	test_check(b, "Allocation B failed");
	test_check(c, "Allocation C failed");
	test_check(d, "Allocation D failed");

	d.~Box();
	test_check(!d, "Allocation D was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		test_check(e.data() == d_ptr, "Top allocation was not reclaimed");
	}

	a.~Box();
	test_check(!a, "Allocation A was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		test_check(e.data() == d_ptr, "Allocation shifted on non-contiguous frees");
	}

	c.~Box();
	test_check(!c, "Allocation C was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		test_check(e.data() == c_ptr, "Top allocation was not reclaimed");
	}

	b.~Box();
	test_check(!b, "Allocation B was not released");

	{
		auto e = stack::Allocator::create<u64>(8);
		test_check(e.data() == a_ptr, "Contiguous frees were not reclaimed");
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
	test_check(box.data() == base_ptr, "Frees were not reclaimed");
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
			test_check(middle[i] == i * 3, "Middle allocation corrupted");
	}

	for (u64 i = 0; i < outer.size(); ++i)
		test_check(outer[i] == i, "Outer allocation corrupted");
}

void test_object_lifetime() {
	Tracker::constructed = 0;
	Tracker::destroyed = 0;

	{
		auto box = stack::Allocator::create<Tracker>(100, 42);

		test_check(
			Tracker::constructed == 100,
			"Incorrect number of constructions"
		);

		for (u64 i = 0; i < box.size(); ++i)
			test_check(box[i].value == 42, "Incorrect constructor argument");
	}

	test_check(
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
		test_check(ba_ptr == base_ptr, "Freed box A was not reclaimed");
		ba.~Box();

		{
			auto b = stack::Allocator::create<u64>(2);
			b[0] = iteration;
			b[1] = iteration + 1;
		}

		auto bb = stack::Allocator::create<u64>(1);
		auto bb_ptr = bb.data();
		test_check(bb_ptr == base_ptr, "Freed box B was not reclaimed");
		bb.~Box();

		{
			auto c = stack::Allocator::create<u64>(3);
			c[0] = iteration;
			c[1] = iteration + 1;
			c[2] = iteration + 2;
		}

		auto bc = stack::Allocator::create<u64>(1);
		auto bc_ptr = bc.data();
		test_check(bc_ptr == base_ptr, "Freed box C was not reclaimed");
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
				test_check(a[i] == (i ^ iteration), "A corrupted");

			for (u64 i = 0; i < b.size(); ++i)
				test_check(b[i] == (i ^ (iteration << 1)), "B corrupted");

			for (u64 i = 0; i < c.size(); ++i)
				test_check(c[i] == (i ^ (iteration << 2)), "C corrupted");
		}
	}
}

}

using namespace descent;
using namespace descent::memory;

static constexpr test::Case cases[] = {
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

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
