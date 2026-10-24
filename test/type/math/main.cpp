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

// math::npow2(x) is the smallest power of two not less than x. When that power
// is not representable in x's type (x > 2^(bits-1)), it returns 0, which no
// valid input produces. npow2(0) is 1.

extern "C" {
#include <stdio.h>
}

#include <harness.hpp>

#include <descent/type/core.hpp>
#include <descent/type/math.hpp>

using namespace descent;

// Usable in constant expressions across the whole domain.
static_assert(math::npow2(u32(0x40000001u)) == 0x80000000u, "u32 top power");
static_assert(math::npow2(u32(0x80000001u)) == 0, "u32 oversize");
static_assert(math::npow2((u64(1) << 40) + 1) == u64(1) << 41, "u64 mid-range");
static_assert(math::npow2(U64_MAX) == 0, "u64 oversize");
static_assert(math::npow2((u128(1) << 100) + 1) == u128(1) << 101, "u128 mid-range");
static_assert(math::npow2(U128_MAX) == 0, "u128 oversize");
static_assert(math::npow2(u8(129)) == 0, "u8 oversize");

// Inputs pass through a volatile so the runtime cases are not constant-folded.
template<typename T>
static T launder(T value) {
	volatile T v = value;
	return v;
}

static void hex(char *out, u128 v) {
	snprintf(out, 40, "0x%llx%016llx", (unsigned long long) (v >> 64), (unsigned long long) v);
}

template<typename T>
static void expect(T input, T expected) {
	const T got = math::npow2(launder(input));
	char in[40], want[40], have[40];
	hex(in, input);
	hex(want, expected);
	hex(have, got);
	test_check(got == expected, "npow2(%s): expected %s, got %s", in, want, have);
}

template<typename T, u32 BITS>
static void boundaries() {
	const T top = T(T(1) << (BITS - 1));
	const T max = T(~T(0));

	expect<T>(0, 1);
	expect<T>(1, 1);
	expect<T>(5, 8);
	expect<T>(T(top / 2 + 1), top);
	expect<T>(top, top);
	expect<T>(T(top + 1), 0);
	expect<T>(max, 0);
}

// Mid-range values that need more than 30 bits of shift.
static void npow2_wide() {
	expect<u64>((u64(1) << 40) + 1, u64(1) << 41);
	expect<u64>(u64(0x80000001u), u64(1) << 32);
	expect<u128>((u128(1) << 100) + 1, u128(1) << 101);
}

static constexpr test::Case cases[] = {
	{ "npow2_u8",   boundaries<u8, 8> },
	{ "npow2_u16",  boundaries<u16, 16> },
	{ "npow2_u32",  boundaries<u32, 32> },
	{ "npow2_u64",  boundaries<u64, 64> },
	{ "npow2_u128", boundaries<u128, 128> },
	{ "npow2_wide", npow2_wide },
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
