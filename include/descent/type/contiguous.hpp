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

#ifndef DESCENT_TYPE_CONTIGUOUS_HPP
#define DESCENT_TYPE_CONTIGUOUS_HPP

#include <descent/type/core.hpp>
#include <descent/type/math.hpp>
#include <descent/type/meta.hpp>

namespace descent {

template<typename T>
class view;

template<typename T, typename Derived>
class contiguous_interface {
	template<typename, typename>
	friend class contiguous_interface;

private:
	constexpr Derived &self() {
		return *static_cast<Derived *>(this);
	}

	constexpr const Derived &self() const {
		return *static_cast<const Derived *>(this);
	}

	constexpr auto s_data() {
		return self().data();
	}

	constexpr auto s_data() const {
		return self().data();
	}

	constexpr u64 s_size() const {
		return self().size();
	}

	constexpr decltype(auto) at(u64 index) {
		return s_data()[index];
	}

	constexpr decltype(auto) at(u64 index) const {
		return s_data()[index];
	}

	template<typename U>
	static constexpr view<U> sub_view(U *data, u64 size, u64 position, u64 count) {
		if (!data || position > size)
			return {};

		if (count > size - position)
			count = size - position;

		return view<U>(data + position, count);
	}

protected:
	contiguous_interface() = default;

public:
	static constexpr u64 INVALID_INDEX = U64_MAX;

	// Operators

	template<typename U, typename Other>
	constexpr bool operator ==(const contiguous_interface<U, Other> &other) const {
		static_assert(
			type::is_same<typename type::remove_cv<T>::type, typename type::remove_cv<U>::type>::value,
			"comparing views of different element types"
		);

		if (s_size() != other.s_size())
			return false;

		if (!__builtin_is_constant_evaluated() && s_data() == other.s_data())
			return true;

		for (u64 i = 0; i < s_size(); ++i)
			if (at(i) != other[i])
				return false;

		return true;
	}

	constexpr decltype(auto) operator [](u64 index) {
		return at(index);
	}

	constexpr decltype(auto) operator [](u64 index) const {
		return at(index);
	}

	// Accessors

	constexpr bool empty() const {
		return s_size() == 0;
	}

	// Utilities

	constexpr bool contains(const T &value) const {
		return find(value) != INVALID_INDEX;
	}

	constexpr u64 find(const T &value, u64 position = 0) const {
		if (!s_data() || position >= s_size())
			return INVALID_INDEX;

		for (u64 i = position; i < s_size(); ++i)
			if (at(i) == value)
				return i;

		return INVALID_INDEX;
	}

	constexpr u64 rfind(const T &value, u64 position = U64_MAX) const {
		if (!s_data() || s_size() == 0)
			return INVALID_INDEX;

		if (position >= s_size())
			position = s_size() - 1;

		for (u64 i = position + 1; i > 0; --i)
			if (at(i - 1) == value)
				return i - 1;

		return INVALID_INDEX;
	}

	constexpr auto sub(u64 position, u64 count = U64_MAX) {
		return sub_view(s_data(), s_size(), position, count);
	}

	constexpr auto sub(u64 position, u64 count = U64_MAX) const {
		return sub_view(s_data(), s_size(), position, count);
	}
};

template<typename T>
class view : public contiguous_interface<T, view<T>> {
private:
	T *_data  = nullptr;
	u64 _size = 0;

public:
	constexpr view() = default;

	constexpr view(T *data, u64 size) : _data(data), _size(data ? size : 0) {}

	constexpr view(const view<typename type::remove_const<T>::type> &other)
	: _data(other.data()), _size(other.size()) {}

	constexpr T *data() const {
		return _data;
	}

	constexpr u64 size() const {
		return _size;
	}
};

template<typename T, u64 SIZE>
class buffer : public contiguous_interface<T, buffer<T, SIZE>> {
	static_assert(SIZE > 0, "Invalid buffer capacity");
private:
	T _data[SIZE] = {};

public:
	constexpr buffer() = default;

	constexpr T *data() {
		return _data;
	}

	constexpr const T *data() const {
		return _data;
	}

	constexpr u64 size() const {
		return SIZE;
	}

	constexpr operator view<T>() {
		return view<T>(_data, SIZE);
	}

	constexpr operator view<const T>() const {
		return view<const T>(_data, SIZE);
	}
};

}

#endif
