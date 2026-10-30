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

#ifndef DESCENT_FS_HPP
#define DESCENT_FS_HPP

#include <descent/type/string.hpp>
#include <descent/type/utility.hpp>

namespace descent::fs {

enum class Access {
	READ = 0,
	EDIT,
};

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

	Handle &operator =(Handle &&other) {
		if (&other != this)
			swap(h, other.h);

		return *this;
	}

	bool valid() const {
		return h >= 0;
	}

	~Handle();
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

	Directory(Directory &&other) : h(move(other.h)) {}

	Directory &operator =(Directory &&other);

	explicit operator bool() const {
		return h.valid();
	}

	const Handle &handle() const {
		return h;
	}

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
	Handle h;

	explicit File(i32 handle) : h(handle) {}

public:
	File() = default;

	File(const Directory &parent, string name, Access a);

	explicit operator bool() const {
		return h.valid();
	}

	const Handle &handle() const {
		return h;
	}
};

bool set_program_name(string name);

}

#endif
