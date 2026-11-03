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

#include <descent/type/core.hpp>
#include <descent/type/math.hpp>
#include <descent/type/utility.hpp>

namespace descent {

template<typename T>
class view;

namespace contiguous {

template<typename T, typename Derived>
class interface {
	interface() = delete;

private:
	constexpr const Derived &self() const {
		return *static_cast<const Derived *>(this);
	}

	constexpr T &at(u64 index) {
		return data()[index];
	}

	constexpr const T &at(u64 index) const {
		return data()[index];
	}

	template<typename U>
	constexpr view<U> sub_view(u64 position, u64 count) { 
		if (position > size() || !data())
			return {};
 
		if (count > size() - position)
			count = size() - position;
 
		return view<U>(data() + position, count);
	}

protected:
	constexpr T *data() {
		return self()->data();
	}

	constexpr const T *data() const {
		return self()->data();
	}

	constexpr u64 size() const {
		return self()->size();
	}

public:
	static constexpr u64 INVALID_INDEX = U64_MAX;

	// Operators

	template<typename U, typename Other>
	bool operator ==(const interface<U, Other> &other) const {
		if constexpr (!type::is_same<T, U>::value)
			return false;

		if (size() != other.size())
			return false;

		if (data() == other.data())
			return true;

		for (u64 i = 0; i < size(); ++i)
			if (at(i) != other[i])
				return false;

		return true;
	}

	constexpr T &operator[](u64 index) {
		return at(index);
	}

	constexpr const T &operator[](u64 index) const {
		return at(index);
	}

	// Accessors

	constexpr bool empty() const {
		return size() == 0;
	}

	// Utilities

	constexpr bool contains(const T &value) const {
		return find(value) != INVALID_INDEX;
	}

	constexpr u64 find(T value, u64 position = 0) const {
		if (!data() || position >= size())
			return INVALID_INDEX;

		for (u64 i = position; i < size(); ++i)
			if (at(i) == value)
				return i;
 
		return INVALID_INDEX;
	}

	constexpr u64 rfind(char value, u64 position = U64_MAX) const {
		if (!data() || size() == 0)
			return INVALID_INDEX;

		if (position >= size())
			position = size() - 1;

		for (u64 i = position + 1; i > 0; --i)
			if (at(i - 1) == value)
				return i - 1;

		return INVALID_INDEX;
	}

	constexpr view<T> sub(u64 pos, u64 count = U64_MAX) const {
		u64 n = size();

		if (pos > n || !data())
			return {};

		if (count > n - pos)
			count = n - pos;

		return string(data() + pos, count);
	}
};

}

template<typename T>
class view : public contiguous::interface<view<T>, T> {
private:
	T *_data;
	u64 _size;

public:
	constexpr T *data() {
		return _data;
	}

	constexpr const T *data() const {
		return _data;
	}

	constexpr u64 size() const {
		return _size;
	}
};

template<typename T, u64 SIZE>
class buffer : public contiguous::interface<buffer<T, SIZE>, T> {
private:
	T _data[SIZE];

public:
	constexpr T *data() {
		return _data;
	}

	constexpr const T *data() const {
		return _data;
	}

	constexpr u64 size() const {
		return SIZE;
	}
};

class string {
private:
	const char *_data;
	u64 _size;

	constexpr char at(u64 index) const {
		return data()[index];
	}

public:
	static constexpr u64 INVALID_INDEX = U64_MAX;

	// Constructors

	constexpr string() : _data(nullptr), _size(0) {}

	constexpr string(const char *data, u64 size) : _data(data), _size(data ? size : 0) {}

	explicit constexpr string(const char *data) : _data(data), _size(0) {
		if (!_data)
			return;

		_size = U64_MAX;

		const u64 LENGTH = c_length();
		if (LENGTH == INVALID_INDEX || LENGTH + 1 == INVALID_INDEX) {
			_data = nullptr;
			_size = 0;
			return;
		}

		_size = LENGTH + 1;
	}

	// Operators

	bool operator ==(string other) const {
		if (size() != other.size())
			return false;

		if (data() == other.data())
			return true;

		for (u64 i = 0; i < size(); ++i)
			if (at(i) != other[i])
				return false;

		return true;
	}

	constexpr char operator [](u64 n) const {
		return at(n);
	}

	constexpr operator bool() const {
		return data() != nullptr;
	}

	// Accessors

	constexpr const char *data() const {
		return _data;
	}

	constexpr u64 size() const {
		return _size;
	}

	constexpr bool empty() const {
		return size() == 0;
	}

	// Utilities

	constexpr bool contains(char c) const {
		return find(c) != INVALID_INDEX;
	}

	constexpr u64 find(char c, u64 pos = 0) const {
		if (!data() || pos >= size())
			return INVALID_INDEX;

		const char *r = __builtin_char_memchr(data() + pos, c, size() - pos);
		return r ? u64(r - data()) : INVALID_INDEX;
	}

	constexpr u64 rfind(char c, u64 pos = U64_MAX) const {
		if (!data() || size() == 0)
			return INVALID_INDEX;

		if (pos >= size())
			pos = size() - 1;

		for (u64 i = pos + 1; i > 0; --i)
			if (at(i - 1) == c)
				return i - 1;

		return INVALID_INDEX;
	}

	constexpr string sub(u64 pos, u64 count = U64_MAX) const {
		u64 n = size();

		if (pos > n || !data())
			return {};

		if (count > n - pos)
			count = n - pos;

		return string(data() + pos, count);
	}

	// C-String Methods

	// Checks if the string contains a valid c-string
	constexpr bool c_safe() const {
		return rfind('\0') != INVALID_INDEX;
	}

	// Returns the length of the first contained c-string
	constexpr u64 c_length() const {
		if (!data())
			return INVALID_INDEX;

		for (u64 i = 0; i < size(); ++i)
			if (at(i) == '\0')
				return i;

		return INVALID_INDEX;
	}

	// Return a string containing only the first contained c-string
	constexpr string c_string() const {
		u64 length = c_length();
		if (length == INVALID_INDEX)
			return {};

		return string(data(), length + 1);
	}
};

}

#endif
