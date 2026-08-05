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

#include <descent/random.h>

#include <math.h>

#include <descent/sys.h>
#include <descent/type/bits.h>
#include <descent/type/core.h>
#include <descent/type/math.h>

static _Thread_local u64 state = 0;

void random_seed(u64 seed) {
	state = seed;
}

bool random_bool(void) {
	return (bool) (random_u64() & 1);
}

u8 random_u8(void) {
	return (u8) random_u64();
}

u8 random_u8_range(u8 min, u8 max) {
	return (u8) random_u64_range((u64) min, (u64) max);
}

i8 random_i8(void) {
	return (i8) random_u64();
}

i8 random_i8_range(i8 min, i8 max) {
	return (i8) random_i64_range((i64) min, (i64) max);
}

u16 random_u16(void) {
	return (u16) random_u64();
}

u16 random_u16_range(u16 min, u16 max) {
	return (u16) random_u64_range((u64) min, (u64) max);
}

i16 random_i16(void) {
	return (i16) random_u64();
}

i16 random_i16_range(i16 min, i16 max) {
	return (i16) random_i64_range((i64) min, (i64) max);
}

u32 random_u32(void) {
	return (u32) random_u64();
}

u32 random_u32_range(u32 min, u32 max) {
	return (u32) random_u64_range((u64) min, (u64) max);
}

i32 random_i32(void) {
	return (i32) random_u64();
}

i32 random_i32_range(i32 min, i32 max) {
	return (i32) random_i64_range((i64) min, (i64) max);
}

u64 random_u64(void) {
	state += 0xA0761D6478BD642Full;

	u64 high;
	u64 low = mul_64(state ^ 0xE7037ED1A0B428DBull, state, &high);

	return (low ^ high);
}

u64 random_u64_range(u64 min, u64 max) {
	sys_assert(min < max, "Minimum must be less than maximum");

	u64 range = max - min - 1;
	if (range == 0) return min;
	u64 mask = U64_MAX >> clz_64(range);

	u64 random;
	do {
		random = random_u64() & mask;
	} while (random > range);

	return min + random;
}

i64 random_i64(void) {
	return (i64) random_u64();
}

i64 random_i64_range(i64 min, i64 max) {
	sys_assert(min < max, "Minimum must be less than maximum");

	u64 range = ((u64) max - (u64) min) - 1;
	if (range == 0) return min;
	u64 mask = U64_MAX >> clz_64(range);

	u64 random;
	do {
		random = random_u64() & mask;
	} while (random > range);

	return min + (i64) random;
}

u128 random_u128(void) {
	return ((u128) random_u64()) | (((u128) random_u64()) << 64);
}

u128 random_u128_range(u128 min, u128 max) {
	sys_assert(min < max, "Minimum must be less than maximum");

	u128 range = max - min - 1;
	if (range == 0) return min;
	u128 mask = U128_MAX >> clz_128(range);

	u128 random;
	do {
		random = random_u128() & mask;
	} while (random > range);

	return min + random;
}

i128 random_i128(void) {
	return (i128) random_u128();
}

i128 random_i128_range(i128 min, i128 max) {
	sys_assert(min < max, "Minimum must be less than maximum");

	u128 range = ((u128) max - (u128) min) - 1;
	if (range == 0) return min;
	u128 mask = U128_MAX >> clz_128(range);

	u128 random;
	do {
		random = random_u128() & mask;
	} while (random > range);

	return min + (i128) random;
}

f32 random_f32(void) {
	union { u32 u; f32 f; } v;
	
	// We use 23 bits of randomness
	v.u = (127ul << 23) | (random_u32() & 0x007FFFFFul);
	v.f -= 1.0f;

	return v.f;
}

f32 random_f32_range(f32 min, f32 max) {
	sys_assert(min < max, "Minimum must be less than maximum");
	return min + (max - min) * random_f32();
}

f32 random_f32_symmetric(void) {
	union { u32 u; f32 f; } v;
	
	// We use 24 bits of randomness
	u32 r = random_u32();

	v.u = (127ul << 23) | (r & 0x007FFFFFul);
	v.f -= 1.0f;
	v.u |= r & 0x80000000ul;

	return v.f;
}

f32 random_f32_symmetric_range(f32 max) {
	return max * random_f32_symmetric();
}

f32 random_f32_normal(f32 mean, f32 std_dev) {
	static _Thread_local f32 n2;
	static _Thread_local bool spare;
	f32 scale, n1, r1, r2;

	if (spare) {
		spare = false;
		return (n2 * std_dev) + mean;
	}

	do {
		r1 = random_f32_symmetric();
		r2 = random_f32_symmetric();
		scale = (r1 * r1) + (r2 * r2);
	} while (scale >= 1.0f || scale == 0.0f);

	scale = sqrtf(-2.0f * logf(scale) / scale);

	n1 = r1 * scale;
	n2 = r2 * scale;
	spare = true;

	return (n1 * std_dev) + mean;
}

f32 random_f32_exponential(f32 lambda) {
	return logf(1.0f - random_f32()) / (-lambda);
}

f64 random_f64(void) {
	union { u64 u; f64 f; } v;

	// We use 52 bits of randomness
	v.u = (1023ull << 52) | (random_u64() & 0xFFFFFFFFFFFFFull);
	v.f -= 1.0;

	return v.f;
}

f64 random_f64_range(f64 min, f64 max) {
	sys_assert(min < max, "Minimum must be less than maximum");
	return min + (max - min) * random_f64();
}

f64 random_f64_symmetric(void) {
	union { u64 u; f64 f; } v;
	
	// We use 53 bits of randomness
	u64 r = random_u64();

	v.u = (1023ull << 52) | (r & 0xFFFFFFFFFFFFFull);
	v.f -= 1.0;
	v.u |= r & 0x8000000000000000ull;

	return v.f;
}

f64 random_f64_symmetric_range(f64 max) {
	return max * random_f64_symmetric();
}

f64 random_f64_normal(f64 mean, f64 std_dev) {
	static _Thread_local f64 n2;
	static _Thread_local bool spare;
	f64 scale, n1, r1, r2;

	if (spare) {
		spare = false;
		return (n2 * std_dev) + mean;
	}

	do {
		r1 = random_f64_symmetric();
		r2 = random_f64_symmetric();
		scale = (r1 * r1) + (r2 * r2);
	} while (scale >= 1.0 || scale == 0.0);

	scale = sqrt(-2.0 * log(scale) / scale);

	n1 = r1 * scale;
	n2 = r2 * scale;
	spare = true;

	return (n1 * std_dev) + mean;
}

f64 random_f64_exponential(f64 lambda) {
	return log(1.0 - random_f64()) / (-lambda);
}

/* TODO
- Ziggurat method for non-uniform distributions
- Test Lemire's fastrange method for PRNG bounding
*/
