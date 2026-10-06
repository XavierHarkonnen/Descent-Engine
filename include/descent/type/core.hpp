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

#ifndef DESCENT_TYPE_CORE_HPP
#define DESCENT_TYPE_CORE_HPP

namespace descent {

using u8    = __UINT8_TYPE__;
using u16   = __UINT16_TYPE__;
using u32   = __UINT32_TYPE__;
using u64   = __UINT64_TYPE__;
using u128  = __uint128_t;

constexpr u8   U8_MAX   = ((u8  )~(u8  )0);
constexpr u16  U16_MAX  = ((u16 )~(u16 )0);
constexpr u32  U32_MAX  = ((u32 )~(u32 )0);
constexpr u64  U64_MAX  = ((u64 )~(u64 )0);
constexpr u128 U128_MAX = ((u128)~(u128)0);

using i8   = __INT8_TYPE__;
using i16  = __INT16_TYPE__;
using i32  = __INT32_TYPE__;
using i64  = __INT64_TYPE__;
using i128 = __int128;

constexpr i8   I8_MAX   = ((i8  )(U8_MAX  >> 1));
constexpr i16  I16_MAX  = ((i16 )(U16_MAX >> 1));
constexpr i32  I32_MAX  = ((i32 )(U32_MAX >> 1));
constexpr i64  I64_MAX  = ((i64 )(U64_MAX >> 1));
constexpr i128 I128_MAX = ((i128)(U128_MAX >> 1));

constexpr i8   I8_MIN   = (-I8_MAX   - 1);
constexpr i16  I16_MIN  = (-I16_MAX  - 1);
constexpr i32  I32_MIN  = (-I32_MAX  - 1);
constexpr i64  I64_MIN  = (-I64_MAX  - 1);
constexpr i128 I128_MIN = (-I128_MAX - 1);

using f32 = float;
using f64 = double;

constexpr f32 F32_MAX = __FLT_MAX__;
constexpr f64 F64_MAX = __DBL_MAX__;

constexpr f32 F32_MIN = __FLT_MIN__;
constexpr f64 F64_MIN = __DBL_MIN__;

constexpr f32 F32_INF = __builtin_inff();
constexpr f64 F64_INF = __builtin_inf();

constexpr f32 F32_NAN = __builtin_nanf("");
constexpr f64 F64_NAN = __builtin_nan("");

constexpr f32 F32_NAN_S = __builtin_nansf("");
constexpr f64 F64_NAN_S = __builtin_nans("");

}

#endif