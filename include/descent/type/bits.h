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

#ifndef DESCENT_TYPE_BITS_H
#define DESCENT_TYPE_BITS_H

#include <descent/type/core.h>

static inline u64 clz_8  (u8   x) { return (u64) __builtin_clzg(x, 8  ); }
static inline u64 clz_16 (u16  x) { return (u64) __builtin_clzg(x, 16 ); }
static inline u64 clz_32 (u32  x) { return (u64) __builtin_clzg(x, 32 ); }
static inline u64 clz_64 (u64  x) { return (u64) __builtin_clzg(x, 64 ); }
static inline u64 clz_128(u128 x) { return (u64) __builtin_clzg(x, 128); }

static inline u64 ctz_8  (u8   x) { return (u64) __builtin_ctzg(x, 8  ); }
static inline u64 ctz_16 (u16  x) { return (u64) __builtin_ctzg(x, 16 ); }
static inline u64 ctz_32 (u32  x) { return (u64) __builtin_ctzg(x, 32 ); }
static inline u64 ctz_64 (u64  x) { return (u64) __builtin_ctzg(x, 64 ); }
static inline u64 ctz_128(u128 x) { return (u64) __builtin_ctzg(x, 128); }

static inline u64 popcnt_8  (u8   x) { return (u64) __builtin_popcountg(x); }
static inline u64 popcnt_16 (u16  x) { return (u64) __builtin_popcountg(x); }
static inline u64 popcnt_32 (u32  x) { return (u64) __builtin_popcountg(x); }
static inline u64 popcnt_64 (u64  x) { return (u64) __builtin_popcountg(x); }
static inline u64 popcnt_128(u128 x) { return (u64) __builtin_popcountg(x); }

static inline bool parity_8  (u8   x) { return (bool) __builtin_popcountg(x) & 1; }
static inline bool parity_16 (u16  x) { return (bool) __builtin_popcountg(x) & 1; }
static inline bool parity_32 (u32  x) { return (bool) __builtin_popcountg(x) & 1; }
static inline bool parity_64 (u64  x) { return (bool) __builtin_popcountg(x) & 1; }
static inline bool parity_128(u128 x) { return (bool) __builtin_popcountg(x) & 1; }

static inline u8   rotl_8  (u8   x, u64 c) { const u64 mask = 8   - 1; c &= mask; return (u8)   ((x << c) | (x >> (-c & mask))); }
static inline u16  rotl_16 (u16  x, u64 c) { const u64 mask = 16  - 1; c &= mask; return (u16)  ((x << c) | (x >> (-c & mask))); }
static inline u32  rotl_32 (u32  x, u64 c) { const u64 mask = 32  - 1; c &= mask; return (u32)  ((x << c) | (x >> (-c & mask))); }
static inline u64  rotl_64 (u64  x, u64 c) { const u64 mask = 64  - 1; c &= mask; return (u64)  ((x << c) | (x >> (-c & mask))); }
static inline u128 rotl_128(u128 x, u64 c) { const u64 mask = 128 - 1; c &= mask; return (u128) ((x << c) | (x >> (-c & mask))); }

static inline u8   rotr_8  (u8   x, u64 c) { const u64 mask = 8   - 1; c &= mask; return (u8)   ((x >> c) | (x << (-c & mask))); }
static inline u16  rotr_16 (u16  x, u64 c) { const u64 mask = 16  - 1; c &= mask; return (u16)  ((x >> c) | (x << (-c & mask))); }
static inline u32  rotr_32 (u32  x, u64 c) { const u64 mask = 32  - 1; c &= mask; return (u32)  ((x >> c) | (x << (-c & mask))); }
static inline u64  rotr_64 (u64  x, u64 c) { const u64 mask = 64  - 1; c &= mask; return (u64)  ((x >> c) | (x << (-c & mask))); }
static inline u128 rotr_128(u128 x, u64 c) { const u64 mask = 128 - 1; c &= mask; return (u128) ((x >> c) | (x << (-c & mask))); }

static inline u8   byteswap_8  (u8   x) { return x; }
static inline u16  byteswap_16 (u16  x) { return __builtin_bswap16 (x); }
static inline u32  byteswap_32 (u32  x) { return __builtin_bswap32 (x); }
static inline u64  byteswap_64 (u64  x) { return __builtin_bswap64 (x); }
static inline u128 byteswap_128(u128 x) { return (u128) byteswap_64((u64) (x >> 64)) | (u128) byteswap_64((u64) x) << 64; }

#endif
