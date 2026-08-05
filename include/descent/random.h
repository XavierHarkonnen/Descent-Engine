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

#ifndef DESCENT_RANDOM
#define DESCENT_RANDOM

#include <descent/type/core.h>

// Seeds the thread's random number generator
void random_seed(u64);

// Generates a random boolean value
bool random_bool(void);

// Generates a random 8-bit unsigned integer
u8 random_u8(void);

// Generates a random 8-bit unsigned integer in the range [min, max)
u8 random_u8_range(u8 min, u8 max);

// Generates a random 8-bit signed integer
i8 random_i8(void);

// Generates a random 8-bit signed integer in the range [min, max)
i8 random_i8_range(i8 min, i8 max);

// Generates a random 16-bit unsigned integer
u16 random_u16(void);

// Generates a random 16-bit unsigned integer in the range [min, max)
u16 random_u16_range(u16 min, u16 max);

// Generates a random 16-bit signed integer
i16 random_i16(void);

// Generates a random 16-bit signed integer in the range [min, max)
i16 random_i16_range(i16 min, i16 max);

// Generates a random 32-bit unsigned integer
u32 random_u32(void);

// Generates a random 32-bit unsigned integer in the range [min, max)
u32 random_u32_range(u32 min, u32 max);

// Generates a random 32-bit signed integer
i32 random_i32(void);

// Generates a random 32-bit signed integer in the range [min, max)
i32 random_i32_range(i32 min, i32 max);

// Generates a random 64-bit unsigned integer
u64 random_u64(void);

// Generates a random 64-bit unsigned integer in the range [min, max)
u64 random_u64_range(u64 min, u64 max);

// Generates a random 64-bit signed integer
i64 random_i64(void);

// Generates a random 64-bit signed integer in the range [min, max)
i64 random_i64_range(i64 min, i64 max);

// Generates a random 128-bit unsigned integer
u128 random_u128(void);

// Generates a random 128-bit unsigned integer in the range [min, max)
u128 random_u128_range(u128 min, u128 max);

// Generates a random 128-bit signed integer
i128 random_i128(void);

// Generates a random 128-bit signed integer in the range [min, max)
i128 random_i128_range(i128 min, i128 max);

// Generates a uniformly-distributed random 32-bit floating point number in the range [0.0, 1.0)
f32 random_f32(void);

// Generates a uniformly-distributed random 32-bit floating point number in the range [min, max)
f32 random_f32_range(f32 min, f32 max);

// Generates a uniformly-distributed random 32-bit floating point number in the range (-1.0, 1.0)
f32 random_f32_symmetric(void);

// Generates a uniformly-distributed random 32-bit floating point number in the range (-max, max)
f32 random_f32_symmetric_range(f32 max);

// Generates a normally-distributed random 32-bit floating point number
f32 random_f32_normal(f32 mean, f32 std_dev);

// Generates an exponentially-distributed random 32-bit floating point number
f32 random_f32_exponential(f32 lambda);

// Generates a uniformly-distributed random 64-bit floating point number in the range [0.0, 1.0)
f64 random_f64(void);

// Generates a uniformly-distributed random 64-bit floating point number in the range [min, max)
f64 random_f64_range(f64 min, f64 max);

// Generates a uniformly-distributed random 64-bit floating point number in the range (-1.0, 1.0)
f64 random_f64_symmetric(void);

// Generates a uniformly-distributed random 64-bit floating point number in the range (-max, max)
f64 random_f64_symmetric_range(f64 max);

// Generates a normally-distributed random 64-bit floating point number
f64 random_f64_normal(f64 mean, f64 std_dev);

// Generates an exponentially-distributed random 64-bit floating point number
f64 random_f64_exponential(f64 lambda);

#endif