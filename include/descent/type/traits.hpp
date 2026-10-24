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
#include <descent/type/string.hpp>

namespace descent::type {

enum class Type : u8 {
	INVALID,
	BOOL,
	U8, U16, U32, U64, U128,
	I8, I16, I32, I64, I128,
	F32, F64,
	STRING
};

template<typename T>
struct default_traits {
	static constexpr T min;
	static constexpr T max;

	static constexpr Type type = Type::INVALID;

	using to_signed = void;
	using to_unsigned = void;

	static constexpr bool is_numeric = false;
	static constexpr bool is_bool = false;
	static constexpr bool is_integer = false;
	static constexpr bool is_float = false;
	static constexpr bool is_string = false;

	static constexpr bool is_signed = false;
	static constexpr bool is_unsigned = false;

	static constexpr u64 bits = sizeof(T) * __CHAR_BIT__;
	static constexpr u64 bytes = sizeof(T);
};

template<typename T>
struct traits : default_traits<T> {};

template<>
struct traits<bool> : default_traits<bool> {
	static constexpr bool is_bool = true;

	static constexpr Type type = Type::BOOL;

	static constexpr bool min = false;
	static constexpr bool max = true;
};

template<>
struct traits<u8> : default_traits<u8> {
	static constexpr u8 min = 0;
	static constexpr u8 max = U8_MAX;

	static constexpr Type type = Type::U8;

	using to_signed = i8;
	using to_unsigned = u8;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_unsigned = true;
};

template<>
struct traits<u16> : default_traits<u16> {
	static constexpr u16 min = 0;
	static constexpr u16 max = U16_MAX;

	static constexpr Type type = Type::U16;

	using to_signed = i16;
	using to_unsigned = u16;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_unsigned = true;
};

template<>
struct traits<u32> : default_traits<u32> {
	static constexpr u32 min = 0;
	static constexpr u32 max = U32_MAX;

	static constexpr Type type = Type::U32;

	using to_signed = i32;
	using to_unsigned = u32;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_unsigned = true;
};

template<>
struct traits<u64> : default_traits<u64> {
	static constexpr u64 min = 0;
	static constexpr u64 max = U64_MAX;

	static constexpr Type type = Type::U64;

	using to_signed = i64;
	using to_unsigned = u64;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_unsigned = true;
};

template<>
struct traits<u128> : default_traits<u128> {
	static constexpr u128 min = 0;
	static constexpr u128 max = U128_MAX;

	static constexpr Type type = Type::U128;

	using to_signed = i128;
	using to_unsigned = u128;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_unsigned = true;
};

template<>
struct traits<i8> : default_traits<i8> {
	static constexpr i8 min = I8_MIN;
	static constexpr i8 max = I8_MAX;

	static constexpr Type type = Type::I8;

	using to_signed = i8;
	using to_unsigned = u8;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_signed = true;
};

template<>
struct traits<i16> : default_traits<i16> {
	static constexpr i16 min = I16_MIN;
	static constexpr i16 max = I16_MAX;

	static constexpr Type type = Type::I16;

	using to_signed = i16;
	using to_unsigned = u16;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_signed = true;
};

template<>
struct traits<i32> : default_traits<i32> {
	static constexpr i32 min = I32_MIN;
	static constexpr i32 max = I32_MAX;

	static constexpr Type type = Type::I32;

	using to_signed = i32;
	using to_unsigned = u32;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_signed = true;
};

template<>
struct traits<i64> : default_traits<i64> {
	static constexpr i64 min = I64_MIN;
	static constexpr i64 max = I64_MAX;

	static constexpr Type type = Type::I64;

	using to_signed = i64;
	using to_unsigned = u64;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_signed = true;
};

template<>
struct traits<i128> : default_traits<i128> {
	static constexpr i128 min = I128_MIN;
	static constexpr i128 max = I128_MAX;

	static constexpr Type type = Type::I128;

	using to_signed = i128;
	using to_unsigned = u128;

	static constexpr bool is_numeric = true;
	static constexpr bool is_integer = true;
	static constexpr bool is_signed = true;
};


template<>
struct traits<f32> : default_traits<f32> {
	static constexpr f32 min = F32_MIN;
	static constexpr f32 max = F32_MAX;

	static constexpr Type type = Type::F32;

	using to_signed = f32;

	static constexpr bool is_numeric = true;
	static constexpr bool is_float = true;
	static constexpr bool is_signed = true;
};

template<>
struct traits<f64> : default_traits<f64> {
	static constexpr f64 min = F64_MIN;
	static constexpr f64 max = F64_MAX;

	static constexpr Type type = Type::F64;

	using to_signed = f64;

	static constexpr bool is_numeric = true;
	static constexpr bool is_float = true;
	static constexpr bool is_signed = true;
};

template<>
struct traits<string> : default_traits<string> {
	static constexpr Type type = Type::STRING;

	static constexpr bool is_string = true;
};

template <typename T>
struct remove_reference {
	using type = T;
};

template <typename T>
struct remove_reference<T&> {
	using type = T;
};

template <typename T>
struct remove_reference<T&&> {
	using type = T;
};

template <typename T>
struct remove_const {
	using type = T;
};

template <typename T>
struct remove_const<const T> {
	using type = T;
};

template <typename T>
struct remove_volatile {
	using type = T;
};

template <typename T>
struct remove_volatile<volatile T> {
	using type = T;
};

template <typename T>
struct decay {
private:
	using unr = typename remove_reference<T>::type;
	using unc = typename remove_const<unr>::type;
	using unv = typename remove_volatile<unc>::type;

public:
	using type = unv;

	static_assert(traits<type>::type != Type::INVALID, "Attempted to decay invalid type");
};

}

#endif
