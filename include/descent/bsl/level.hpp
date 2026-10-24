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

#ifndef DESCENT_BSL_LEVEL_HPP
#define DESCENT_BSL_LEVEL_HPP

#include <descent/bsl/serial.hpp>
#include <descent/type/core.hpp>

namespace descent::bsl {

enum class Level : u8 {
	TRACE   = u8(1) << 0, // Log for developers: fine-grained event and process information.
	DEBUG   = u8(1) << 1, // Log for developers: debug event information.
	INFO    = u8(1) << 2, // Log for operators: noteworthy event information.
	WARN    = u8(1) << 3, // Log for operators: unusual or abnormal state, recoverable.
	ERROR   = u8(1) << 4, // Log for operators: error state, possibly recoverable.
	FATAL   = u8(1) << 5, // Log for operators: severe error state, not recoverable.
	NONE    = u8(0x00),
	DEFAULT = u8(0x3C),
	ALL     = u8(0x3F),
};

constexpr Level operator ~(const Level &l) {
	return Level(~u8(l));
}

constexpr Level operator &(const Level &rhs, const Level &lhs) {
	return Level(u8(rhs) & u8(lhs));
}

constexpr Level operator |(const Level &rhs, const Level &lhs) {
	return Level(u8(rhs) | u8(lhs));
}

constexpr Level operator &=(Level &rhs, const Level &lhs) {
	return (rhs = (rhs & lhs));
}

constexpr Level operator |=(Level &rhs, const Level &lhs) {
	return (rhs = (rhs | lhs));
}

inline bool serialize(const Level &level, u8* output, u64 size, u64 &offset) {
	return serialize(u8(level), output, size, offset);
}

inline bool deserialize(Level &level, const u8* input, u64 size, u64 &offset) {
	u8 value;
	if (!deserialize(value, input, size, offset))
		return false;

	level = Level(value);

	return true;
}

}

#endif
