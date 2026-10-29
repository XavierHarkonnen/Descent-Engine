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

#ifndef DESCENT_LIB_FILE_UTILS_HPP
#define DESCENT_LIB_FILE_UTILS_HPP

#include <descent/type/core.hpp>
#include <descent/type/string.hpp>
#include <descent/type/math.hpp>

#include "file.hpp"

namespace descent {

static constexpr u64 MAX_NAME_LENGTH = 128;

inline bool validate_name(string name) {
	if (!name)
		return false;

	const u64 SIZE = math::min(MAX_NAME_LENGTH + 1, name.size());

	u64 length;
	for (length = 0; length < SIZE; ++length) {
		u8 c = u8(name[length]);

		if (c == 0)
			break;

		if (c <= 31 || c >= 127 || c == '/')
			return false;
	}

	if (length > MAX_NAME_LENGTH)
		return false;

	if (length == 0)
		return false;

	if (length == 1 && name[0] == '.')
		return false;

	if (length == 2 && name[0] == '.' && name[1] == '.')
		return false;

	return true;
}

inline bool validate_path(const Directory &root, string name) {
	return root && validate_name(name);
}

}

#endif
