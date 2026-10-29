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

namespace descent {

class Directory {
	friend class File;

	Directory(const Directory &)             = delete;
	Directory &operator =(const Directory &) = delete;

private:
	int fd = -1;

	explicit Directory(int fd) : fd(fd) {}

public:
	Directory() = default;

	Directory(const Directory &parent, string name);

	Directory(Directory &&other) : fd(other.fd) {
		other.fd = -1;
	}

	Directory &operator =(Directory &&other);

	explicit operator bool() const {
		return fd >= 0;
	}

	~Directory();

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
	int fd = -1;
	explicit File(int fd) : fd(fd) {}

public:
	File() = default;

	File(const Directory &parent, const char *name);
};

bool set_program_name(string name);

}

#endif
