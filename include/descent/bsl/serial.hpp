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

#ifndef DESCENT_BSL_SERIAL_HPP
#define DESCENT_BSL_SERIAL_HPP

#include <descent/type/core.hpp>
#include <descent/type/string.hpp>
#include <descent/type/traits.hpp>

namespace descent::bsl {

template<typename T>
inline bool serialize(const T &value, u8 *output, u64 size, u64 &offset) {
	static_assert(type::traits<T>::type != type::Type::INVALID, "Attempted to serialize invalid type");
	static_assert(type::traits<T>::type != type::Type::STRING, "Attempted to serialize string");

	if (!output || offset >= size || size - offset < sizeof(T))
		return false;

	__builtin_memcpy(output + offset, &value, sizeof(T));
	offset += sizeof(T);

	return true;
}

template<typename T>
bool deserialize(T &value, const u8 *input, u64 size, u64 &offset) {
	static_assert(type::traits<T>::type != type::Type::INVALID, "Attempted to deserialize invalid type");
	static_assert(type::traits<T>::type != type::Type::STRING, "Attempted to deserialize string");

	if (!input || offset >= size || size - offset < sizeof(T))
		return false;

	__builtin_memcpy(&value, input + offset, sizeof(T));
	offset += sizeof(T);

	return true;
}

inline bool serialize(const type::Type &type, u8 *output, u64 size, u64 &offset) {
	return serialize(u8(type), output, size, offset);
}

inline bool deserialize(type::Type &type, const u8 *input, u64 size, u64 &offset) {
	u8 value;
	if (!deserialize(value, input, size, offset))
		return false;

	type = type::Type(value);

	return true;
}

inline bool serialize(const string &value, u8 *output, u64 size, u64 &offset) {
	u64 cursor       = offset;
	const u64 length = value.size();

	if (!serialize(length, output, size, cursor))
		return false;

	if (cursor > size || length > size - cursor)
		return false;

	if (length != 0)
		__builtin_memcpy(output + cursor, value.data(), length);

	cursor += length;
	offset = cursor;

	return true;
}

inline bool deserialize(string &value, const u8 *input, u64 size, u64 &offset) {
	u64 cursor = offset;
	u64 length;

	if (!deserialize(length, input, size, cursor))
		return false;

	if (cursor > size || length > size - cursor)
		return false;

	const char *data = reinterpret_cast<const char *>(input + cursor);

	value = string(data, length);

	cursor += length;
	offset = cursor;

	return true;
}

}

#endif
