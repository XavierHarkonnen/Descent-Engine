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

#ifndef DESCENT_LIB_FS_DIRECTORY_SYSTEM_HPP
#define DESCENT_LIB_FS_DIRECTORY_SYSTEM_HPP

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <descent/type/core.hpp>
#include <descent/memory/stack.hpp>

namespace descent::fs::directory {

static constexpr i32 MODE = 0700;

enum class Links : bool {
	REFUSE = false,
	FOLLOW
};

constexpr i32 flags(Links l) {
	return O_PATH | O_DIRECTORY | O_CLOEXEC | (l == Links::REFUSE ? O_NOFOLLOW : 0);
}

inline i32 wrap_open(const char *name, Links l) {
	i32 fd;
	do {
		fd = ::open(name, flags(l), MODE);
	} while (fd < 0 && errno == EINTR);

	return fd;
}

inline i32 wrap_open(i32 root, const char *name, Links l) {
	i32 fd;
	do {
		fd = ::openat(root, name, flags(l), MODE);
	} while (fd < 0 && errno == EINTR);

	return fd;
}

inline void wrap_mkdir(const char *path) {
	int r;
	do {
		r = ::mkdir(path, MODE);
	} while (r < 0 && errno == EINTR);
}

inline void wrap_mkdir(i32 root, const char *path) {
	int r;
	do {
		r = ::mkdirat(root, path, MODE);
	} while (r < 0 && errno == EINTR);
}

inline i32 open(i32 root, const char *name) {
	i32 fd = wrap_open(root, name, Links::REFUSE);
	if (fd >= 0 || errno != ENOENT)
		return fd;

	wrap_mkdir(root, name);
	return wrap_open(root, name, Links::REFUSE);
}

inline i32 open(string<> path) {
	if (path.empty() || path[0] != '/')
		return -1;

	bool c_safe = path.c_safe();

	if (c_safe) {
		i32 fd = wrap_open(path.data(), Links::FOLLOW);
		if (fd >= 0 || errno != ENOENT)
			return fd;
	}

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return -1;

	if (!c_safe) {
		i32 fd = wrap_open(buffer.data(), Links::FOLLOW);
		if (fd >= 0 || errno != ENOENT)
			return fd;
	}

	for (u64 i = 1; true; ++i) {
		if (buffer[i] == '\0') {
			if (buffer[i - 1] != '/')
				wrap_mkdir(buffer.data());

			break;
		}

		if (buffer[i] == '/' && buffer[i - 1] != '/') {
			buffer[i] = '\0';
			wrap_mkdir(buffer.data());
			buffer[i] = '/';
		}
	}

	return wrap_open(buffer.data(), Links::FOLLOW);
}

inline i32 open(i32 root, string<> path) {
	if (path.empty() || path[0] == '\0' || path[0] == '/')
		return -1;

	bool c_safe = path.c_safe();

	if (c_safe) {
		i32 fd = wrap_open(root, path.data(), Links::FOLLOW);
		if (fd >= 0 || errno != ENOENT)
			return fd;
	}

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return -1;

	if (!c_safe) {
		i32 fd = wrap_open(root, buffer.data(), Links::FOLLOW);
		if (fd >= 0 || errno != ENOENT)
			return fd;
	}

	for (u64 i = 1; true; ++i) {
		if (buffer[i] == '\0') {
			if (buffer[i - 1] != '/')
				wrap_mkdir(root, buffer.data());

			break;
		}

		if (buffer[i] == '/' && buffer[i - 1] != '/') {
			buffer[i] = '\0';
			wrap_mkdir(root, buffer.data());
			buffer[i] = '/';
		}
	}

	return wrap_open(root, buffer.data(), Links::FOLLOW);
}

}

#endif
