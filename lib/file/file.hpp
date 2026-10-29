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

#ifndef DESCENT_LIB_FILE_FILE_HPP
#define DESCENT_LIB_FILE_FILE_HPP

#include <descent/type/string.hpp>

#include <descent/type/traits.hpp>

namespace descent::fs {

class Directory;

class Handle {
	friend class Directory;
	friend class File;

	Handle(const Handle &)             = delete;
	Handle &operator =(const Handle &) = delete;

private:
	i32 h = -1;

	Handle() = default;

	Handle(i32 handle) : h(handle) {}

	Handle(Handle &&other) : h(other.h) {
		other.h = -1;
	}

	bool is_valid() {
		return h >= 0;
	}
};

class Directory {
	Directory(const Directory &)             = delete;
	Directory &operator =(const Directory &) = delete;

private:
	Handle h;

	explicit Directory(i32 handle) : h(handle) {}

public:
	Directory() = default;

	Directory(const Directory &parent, string name);

	Directory(Directory &&other) : h(other.h) {
		other.handle = INVALID_HANDLE;
	}

	~Directory();

	Directory &operator =(Directory &&other);

	explicit operator bool() const {
		return 
		return handle != INVALID_HANDLE;
	}

	Handle

	static const Directory &base();
	static const Directory &config();
	static const Directory &data();
	static const Directory &state();
	static const Directory &cache();
};

class File {
	File(const File &)             = delete;
	File &operator =(const File &) = delete;

private:
	explicit File(u64 handle) : Handle(handle) {}

public:
	File() = default;

	File(const Directory &parent, string name);
};

bool set_program_name(string name);

}

#endif
