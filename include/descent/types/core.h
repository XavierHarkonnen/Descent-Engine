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

#ifndef DESCENT_TYPES_CORE_H
#define DESCENT_TYPES_CORE_H

#include <descent/build.h>

// Booleans

#define false 0
#define true  1

typedef _Bool  bool;

// Unsigned Integers

typedef __UINT8_TYPE__   u8;
typedef __UINT16_TYPE__  u16;
typedef __UINT32_TYPE__  u32;
typedef __UINT64_TYPE__  u64;
typedef __uint128_t      u128;

#define U8_MAX    ((u8  )~(u8  )0)
#define U16_MAX   ((u16 )~(u16 )0)
#define U32_MAX   ((u32 )~(u32 )0)
#define U64_MAX   ((u64 )~(u64 )0)
#define U128_MAX  ((u128)~(u128)0)

// Signed Integers

typedef __INT8_TYPE__   i8;
typedef __INT16_TYPE__  i16;
typedef __INT32_TYPE__  i32;
typedef __INT64_TYPE__  i64;
typedef __int128_t      i128;

#define I8_MAX    ((i8  )(U8_MAX  >> 1))
#define I16_MAX   ((i16 )(U16_MAX >> 1))
#define I32_MAX   ((i32 )(U32_MAX >> 1))
#define I64_MAX   ((i64 )(U64_MAX >> 1))
#define I128_MAX  ((i128)(U128_MAX >> 1))

#define I8_MIN    (-I8_MAX   - 1)
#define I16_MIN   (-I16_MAX  - 1)
#define I32_MIN   (-I32_MAX  - 1)
#define I64_MIN   (-I64_MAX  - 1)
#define I128_MIN  (-I128_MAX - 1)

// Floating Point Numbers

#define F32_MAX   __FLT_MAX__
#define F64_MAX   __DBL_MAX__

#define F32_MIN   __FLT_MIN__
#define F64_MIN   __DBL_MIN__

#define F32_INF   __builtin_inff()
#define F64_INF   __builtin_inf()

#define F32_NAN   __builtin_nanf("")
#define F64_NAN   __builtin_nan("")

#define F32_NAN_S   __builtin_nansf("")
#define F64_NAN_S   __builtin_nans("")

typedef float   f32;
typedef double  f64;

#endif
