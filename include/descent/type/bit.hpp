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

#ifndef DESCENT_TYPE_BIT_HPP
#define DESCENT_TYPE_BIT_HPP

#include <descent/type/core.hpp>

namespace descent::bit {

constexpr u64 clz(u8   x) { return (u64) __builtin_clzg(x, 8  ); }
constexpr u64 clz(u16  x) { return (u64) __builtin_clzg(x, 16 ); }
constexpr u64 clz(u32  x) { return (u64) __builtin_clzg(x, 32 ); }
constexpr u64 clz(u64  x) { return (u64) __builtin_clzg(x, 64 ); }
constexpr u64 clz(u128 x) { return (u64) __builtin_clzg(x, 128); }

constexpr u64 ctz(u8   x) { return (u64) __builtin_ctzg(x, 8  ); }
constexpr u64 ctz(u16  x) { return (u64) __builtin_ctzg(x, 16 ); }
constexpr u64 ctz(u32  x) { return (u64) __builtin_ctzg(x, 32 ); }
constexpr u64 ctz(u64  x) { return (u64) __builtin_ctzg(x, 64 ); }
constexpr u64 ctz(u128 x) { return (u64) __builtin_ctzg(x, 128); }

constexpr u64 popcnt(u8   x) { return (u64) __builtin_popcountg(x); }
constexpr u64 popcnt(u16  x) { return (u64) __builtin_popcountg(x); }
constexpr u64 popcnt(u32  x) { return (u64) __builtin_popcountg(x); }
constexpr u64 popcnt(u64  x) { return (u64) __builtin_popcountg(x); }
constexpr u64 popcnt(u128 x) { return (u64) __builtin_popcountg(x); }

constexpr bool parity(u8   x) { return (bool) (__builtin_popcountg(x) & 1); }
constexpr bool parity(u16  x) { return (bool) (__builtin_popcountg(x) & 1); }
constexpr bool parity(u32  x) { return (bool) (__builtin_popcountg(x) & 1); }
constexpr bool parity(u64  x) { return (bool) (__builtin_popcountg(x) & 1); }
constexpr bool parity(u128 x) { return (bool) (__builtin_popcountg(x) & 1); }

constexpr u8   rotl(u8   x, u64 c) { const u64 mask = 8   - 1; c &= mask; return (u8)   ((x << c) | (x >> (-c & mask))); }
constexpr u16  rotl(u16  x, u64 c) { const u64 mask = 16  - 1; c &= mask; return (u16)  ((x << c) | (x >> (-c & mask))); }
constexpr u32  rotl(u32  x, u64 c) { const u64 mask = 32  - 1; c &= mask; return (u32)  ((x << c) | (x >> (-c & mask))); }
constexpr u64  rotl(u64  x, u64 c) { const u64 mask = 64  - 1; c &= mask; return (u64)  ((x << c) | (x >> (-c & mask))); }
constexpr u128 rotl(u128 x, u64 c) { const u64 mask = 128 - 1; c &= mask; return (u128) ((x << c) | (x >> (-c & mask))); }

constexpr u8   rotr(u8   x, u64 c) { const u64 mask = 8   - 1; c &= mask; return (u8)   ((x >> c) | (x << (-c & mask))); }
constexpr u16  rotr(u16  x, u64 c) { const u64 mask = 16  - 1; c &= mask; return (u16)  ((x >> c) | (x << (-c & mask))); }
constexpr u32  rotr(u32  x, u64 c) { const u64 mask = 32  - 1; c &= mask; return (u32)  ((x >> c) | (x << (-c & mask))); }
constexpr u64  rotr(u64  x, u64 c) { const u64 mask = 64  - 1; c &= mask; return (u64)  ((x >> c) | (x << (-c & mask))); }
constexpr u128 rotr(u128 x, u64 c) { const u64 mask = 128 - 1; c &= mask; return (u128) ((x >> c) | (x << (-c & mask))); }

constexpr u8   byteswap(u8   x) { return x; }
constexpr u16  byteswap(u16  x) { return __builtin_bswap16 (x); }
constexpr u32  byteswap(u32  x) { return __builtin_bswap32 (x); }
constexpr u64  byteswap(u64  x) { return __builtin_bswap64 (x); }
constexpr u128 byteswap(u128 x) { return (u128) byteswap((u64) (x >> 64)) | (u128) byteswap((u64) x) << 64; }

}

#endif