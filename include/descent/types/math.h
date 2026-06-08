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

#ifndef DESCENT_TYPES_MATH_H
#define DESCENT_TYPES_MATH_H

#include <descent/types/bits.h>
#include <descent/types/core.h>

static inline u8   log2_8  (u8   x) { return 7   - (u8)   clz_8  (x); }
static inline u16  log2_16 (u16  x) { return 15  - (u16)  clz_16 (x); }
static inline u32  log2_32 (u32  x) { return 31  - (u32)  clz_32 (x); }
static inline u64  log2_64 (u64  x) { return 63  - (u64)  clz_64 (x); }
static inline u128 log2_128(u128 x) { return 127 - (u128) clz_128(x); }

static inline bool add_overflow_8  (u8   x, u8   y, u8   *out) { return __builtin_add_overflow(x, y, out); }
static inline bool add_overflow_16 (u16  x, u16  y, u16  *out) { return __builtin_add_overflow(x, y, out); }
static inline bool add_overflow_32 (u32  x, u32  y, u32  *out) { return __builtin_add_overflow(x, y, out); }
static inline bool add_overflow_64 (u64  x, u64  y, u64  *out) { return __builtin_add_overflow(x, y, out); }
static inline bool add_overflow_128(u128 x, u128 y, u128 *out) { return __builtin_add_overflow(x, y, out); }

static inline bool sub_overflow_8  (u8   x, u8   y, u8   *out) { return __builtin_sub_overflow(x, y, out); }
static inline bool sub_overflow_16 (u16  x, u16  y, u16  *out) { return __builtin_sub_overflow(x, y, out); }
static inline bool sub_overflow_32 (u32  x, u32  y, u32  *out) { return __builtin_sub_overflow(x, y, out); }
static inline bool sub_overflow_64 (u64  x, u64  y, u64  *out) { return __builtin_sub_overflow(x, y, out); }
static inline bool sub_overflow_128(u128 x, u128 y, u128 *out) { return __builtin_sub_overflow(x, y, out); }

static inline u8   mul_8  (u8   x, u8   y, u8   *high) { u16  result = (u16)  x * (u16)  y; *high = (u8)  (result >> 8 ); return (u8)  result; }
static inline u16  mul_16 (u16  x, u16  y, u16  *high) { u32  result = (u32)  x * (u32)  y; *high = (u16) (result >> 16); return (u16) result; }
static inline u32  mul_32 (u32  x, u32  y, u32  *high) { u64  result = (u64)  x * (u64)  y; *high = (u32) (result >> 32); return (u32) result; }
static inline u64  mul_64 (u64  x, u64  y, u64  *high) { u128 result = (u128) x * (u128) y; *high = (u64) (result >> 64); return (u64) result; }
static inline u128 mul_128(u128 x, u128 y, u128 *high) {
	u128 x_l = x & (u128) U64_MAX;
	u128 x_h = x >> 64;
	u128 y_l = y & (u128) U64_MAX;
	u128 y_h = y >> 64;

	u128 ll = x_l * y_l;
	u128 lh = x_l * y_h;
	u128 hl = x_h * y_l;
	u128 hh = x_h * y_h;

	u128 c = 0;
	u128 l = ll;
	c += add_overflow_128(l, (u128) (lh << 64), &l);
	c += add_overflow_128(l, (u128) (hl << 64), &l);
	u128 h = hh + (u128) (lh >> 64) + (u128) (hl >> 64) + c;

	*high = h;
	return l;
}

#endif
