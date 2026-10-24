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

namespace descent {

class string {
private:
	const char* _data;
	u64 _size;

	constexpr char at(u64 index) const {
		return data()[index];
	}

public:
	static constexpr u64 INVALID = U64_MAX;

	// Constructors

	constexpr string()
	: _data(nullptr), _size(0) {}

	constexpr string(const char *data, u64 size)
	: _data(data), _size(data ? size : 0) {}

	template<u64 N>
	explicit constexpr string(const char (&data)[N])
	: _data(data), _size(N - 1) {}

	explicit constexpr string(const char *data)
	: _data(data), _size(U64_MAX) {
		_size = c_length();
		if (_size == INVALID)
			_size = 0;
	}

	// Operators

	constexpr bool operator ==(string other) const {
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

	u64 find(char c, u64 pos = 0) const {
		if (!data() || pos >= size())
			return INVALID;

		const void *r = __builtin_memchr(data() + pos, c, size() - pos);
		return r ? u64(static_cast<const char *>(r) - data()) : INVALID;

		return INVALID;
	}

	u64 rfind(char c, u64 pos = U64_MAX) const {
		if (!data() || size() == 0)
			return INVALID;

		if (pos >= size())
			pos = size() - 1;

		for (u64 i = pos + 1; i > 0; --i)
			if (at(i - 1) == c)
				return i - 1;

		return INVALID;
	}

	string sub(u64 pos, u64 count = U64_MAX) const {
		u64 n = size();

		if (pos > n || !data())
			return {};

		if (count > n - pos)
			count = n - pos;

		return string(data() + pos, count);
	}

	// C-String Methods

	// Checks if the string contains a valid c-string
	bool c_safe() const {
		return rfind('\0') != INVALID;
	}

	// Returns the length of the first contained c-string
	u64 c_length() const {
		if (!data())
			return INVALID;

		for (u64 i = 0; i < size(); ++i)
			if (at(i) == '\0')
				return i;

		return INVALID;
	}

	// Return a string containing only the first contained c-string
	string c_string() const {
		u64 length = c_length();
		if (length == INVALID)
			return {};

		return string(data(), length + 1);
	}
};

}

#endif
