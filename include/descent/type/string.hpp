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

#ifndef DESCENT_TYPE_STRING_HPP
#define DESCENT_TYPE_STRING_HPP

#include <descent/type/contiguous.hpp>
#include <descent/type/core.hpp>
#include <descent/type/math.hpp>
#include <descent/type/meta.hpp>

namespace descent {

template<u64 SIZE = 0>
class string;

template<>
class string<0>;

template<typename T, typename Derived>
class string_interface : public contiguous_interface<T, Derived> {
protected:
	string_interface() = default;

public:
	constexpr bool c_safe() const {
		return this->rfind('\0');
	}

	constexpr u64 c_length() const {
		return this->find('\0');
	}

	constexpr string<> c_string() const;
};

template<>
class string<0> : public view<const char> {
public:
	constexpr string() = default;

	constexpr string(const char *data, u64 size) : view(data, size) {}

	explicit constexpr string(const char *data);
};

template<u64 SIZE>
class string : buffer<char, SIZE> {
	static_assert(SIZE > 0, "Invalid string capacity");

public:
	constexpr string() = default;

	constexpr string(const char *data, u64 size);
};

template<typename T, typename Derived>
constexpr string<> string_interface<T, Derived>::c_string() const {
	const u64 length = c_length();
	if (length == this->INVALID_INDEX)
		return {};

	return string<>(this->s_data(), length + 1);
}

}

#endif
