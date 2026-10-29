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

#ifndef DESCENT_TYPE_TRAITS_HPP
#define DESCENT_TYPE_TRAITS_HPP

#include <descent/type/core.hpp>
#include <descent/type/meta.hpp>
#include <descent/type/string.hpp>

namespace descent::type {

enum class Type : u8 {
	BOOL,
	U8,
	U16,
	U32,
	U64,
	U128,
	I8,
	I16,
	I32,
	I64,
	I128,
	F32,
	F64,
	STRING
};

namespace detail {

template<typename T>
struct traits_impl;

struct unsupported {};

struct category_traits {
	static constexpr bool is_numeric = false;
	static constexpr bool is_bool    = false;
	static constexpr bool is_integer = false;
	static constexpr bool is_float   = false;
	static constexpr bool is_string  = false;

	static constexpr bool is_signed   = false;
	static constexpr bool is_unsigned = false;
};

template<typename T>
struct sized_traits : category_traits {
	static constexpr u64 bits  = sizeof(T) * __CHAR_BIT__;
	static constexpr u64 bytes = sizeof(T);
};

template<typename T, typename S>
struct unsigned_traits : sized_traits<T> {
	using to_signed   = S;
	using to_unsigned = T;

	static constexpr bool is_numeric  = true;
	static constexpr bool is_integer  = true;
	static constexpr bool is_unsigned = true;
};

template<typename T, typename U>
struct signed_traits : sized_traits<T> {
	using to_signed   = T;
	using to_unsigned = U;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_signed  = true;
};

template<typename T, typename B, u64 MANTISSA_DIGITS, i64 MAX_EXPONENT>
struct float_traits : sized_traits<T> {
	using to_bits = B;

	static constexpr u64 mantissa_bits = MANTISSA_DIGITS - 1;
	static constexpr u64 exponent_bits = sized_traits<T>::bits - mantissa_bits - 1;
	static constexpr i64 exponent_bias = MAX_EXPONENT - 1;

	static constexpr bool is_numeric = true;
	static constexpr bool is_float   = true;
	static constexpr bool is_signed  = true;
};

template<>
struct traits_impl<bool> : sized_traits<bool> {
	static constexpr Type type = Type::BOOL;

	static constexpr bool min = false;
	static constexpr bool max = true;

	static constexpr bool is_bool = true;
};

template<>
struct traits_impl<u8> : unsigned_traits<u8, i8> {
	static constexpr Type type = Type::U8;

	static constexpr u8 min = 0;
	static constexpr u8 max = U8_MAX;
};

template<>
struct traits_impl<u16> : unsigned_traits<u16, i16> {
	static constexpr Type type = Type::U16;

	static constexpr u16 min = 0;
	static constexpr u16 max = U16_MAX;
};

template<>
struct traits_impl<u32> : unsigned_traits<u32, i32> {
	static constexpr Type type = Type::U32;

	static constexpr u32 min = 0;
	static constexpr u32 max = U32_MAX;
};

template<>
struct traits_impl<u64> : unsigned_traits<u64, i64> {
	static constexpr Type type = Type::U64;

	static constexpr u64 min = 0;
	static constexpr u64 max = U64_MAX;
};

template<>
struct traits_impl<u128> : unsigned_traits<u128, i128> {
	static constexpr Type type = Type::U128;

	static constexpr u128 min = 0;
	static constexpr u128 max = U128_MAX;
};

template<>
struct traits_impl<i8> : signed_traits<i8, u8> {
	static constexpr Type type = Type::I8;

	static constexpr i8 min = I8_MIN;
	static constexpr i8 max = I8_MAX;
};

template<>
struct traits_impl<i16> : signed_traits<i16, u16> {
	static constexpr Type type = Type::I16;

	static constexpr i16 min = I16_MIN;
	static constexpr i16 max = I16_MAX;
};

template<>
struct traits_impl<i32> : signed_traits<i32, u32> {
	static constexpr Type type = Type::I32;

	static constexpr i32 min = I32_MIN;
	static constexpr i32 max = I32_MAX;
};

template<>
struct traits_impl<i64> : signed_traits<i64, u64> {
	static constexpr Type type = Type::I64;

	static constexpr i64 min = I64_MIN;
	static constexpr i64 max = I64_MAX;
};

template<>
struct traits_impl<i128> : signed_traits<i128, u128> {
	static constexpr Type type = Type::I128;

	static constexpr i128 min = I128_MIN;
	static constexpr i128 max = I128_MAX;
};

template<>
struct traits_impl<f32> : float_traits<f32, u32, __FLT_MANT_DIG__, __FLT_MAX_EXP__> {
	static constexpr Type type = Type::F32;

	static constexpr f32 min = -F32_MAX;
	static constexpr f32 max = F32_MAX;

	static constexpr f32 min_normal    = F32_MIN;
	static constexpr f32 min_subnormal = __FLT_DENORM_MIN__;
	static constexpr f32 epsilon       = __FLT_EPSILON__;

	static constexpr f32 infinity      = F32_INF;
	static constexpr f32 nan           = F32_NAN;
	static constexpr f32 signaling_nan = F32_NAN_S;
};

template<>
struct traits_impl<f64> : float_traits<f64, u64, __DBL_MANT_DIG__, __DBL_MAX_EXP__> {
	static constexpr Type type = Type::F64;

	static constexpr f64 min = -F64_MAX;
	static constexpr f64 max = F64_MAX;

	static constexpr f64 min_normal    = F64_MIN;
	static constexpr f64 min_subnormal = __DBL_DENORM_MIN__;
	static constexpr f64 epsilon       = __DBL_EPSILON__;

	static constexpr f64 infinity      = F64_INF;
	static constexpr f64 nan           = F64_NAN;
	static constexpr f64 signaling_nan = F64_NAN_S;
};

template<>
struct traits_impl<string> : category_traits {
	static constexpr Type type = Type::STRING;

	static constexpr bool is_string = true;
};

}

template<typename T, typename = void>
struct is_supported {
	static constexpr bool value = false;
};

template<typename T>
struct is_supported<T, void_t<decltype(sizeof(detail::traits_impl<typename remove_cv<T>::type>))>> {
	static constexpr bool value = true;
};

namespace detail {
template<typename T>
using traits_base =
	typename conditional<is_supported<T>::value, traits_impl<typename remove_cv<T>::type>, unsupported>::type;
}

template<typename T>
struct traits : detail::traits_base<T> {
	static_assert(is_supported<T>::value, "Attempted to use traits on unsupported type");
};

}

#endif
