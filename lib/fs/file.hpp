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

#ifndef DESCENT_LIB_FS_FILE_HPP
#define DESCENT_LIB_FS_FILE_HPP

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
}

#if defined(__has_include)
#if __has_include(<linux/openat2.h>)
extern "C" {
#include <linux/openat2.h>
}
#endif
#endif

#include <descent/access.hpp>
#include <descent/memory/stack.hpp>
#include <descent/type/core.hpp>

#include "data.hpp"

#if defined(SYS_openat2) && defined(OPENAT2_REGULAR) && defined(EFTYPE)
#define DESCENT_FS_FILE_COMPILE_OPENAT2
#endif

namespace descent::fs::file {

constexpr i32 flags(Access a) {
	if (a == Access::EDIT)
		return O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW;

	return O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
}

constexpr i32 mode(Access a) {
	return a == Access::EDIT ? 0600 : 0;
}

inline void wrap_close(i32 fd) {
	const int error = errno;
	::close(fd);
	errno = error;
}

// Kernel 7.2
inline Data wrap_openat2(i32 root, const char *name, Access a) {
#if defined(DESCENT_FS_FILE_COMPILE_OPENAT2)
	struct open_how how = {};

	how.flags = static_cast<u64>(flags(a)) | OPENAT2_REGULAR;
	how.mode  = static_cast<u64>(mode(a));

	long result;
	do {
		result = ::syscall(SYS_openat2, root, name, &how, sizeof(how));
	} while (result < 0 && errno == EINTR);

	i32 fd = static_cast<i32>(result);
	if (fd < 0)
		return {};

	struct stat st;
	if (::fstat(fd, &st) < 0) {
		wrap_close(fd);
		return {};
	}

	return Data(fd, u64(st.st_dev), u64(st.st_ino), u64(st.st_size));

#else
	(void) root;
	(void) name;
	(void) a;

	errno = ENOSYS;

	return {};
#endif
}

inline Data wrap_openat(i32 root, const char *name, Access a) {
	i32 fd;
	do {
		fd = ::openat(root, name, flags(a) | O_NONBLOCK | O_NOCTTY, mode(a));
	} while (fd < 0 && errno == EINTR);

	if (fd < 0)
		return {};

	struct stat st;
	if (::fstat(fd, &st) < 0) {
		wrap_close(fd);
		return {};
	}

	if (!S_ISREG(st.st_mode)) {
		wrap_close(fd);
		errno = EFTYPE;
		return {};
	}

	return Data(fd, u64(st.st_dev), u64(st.st_ino), u64(st.st_size));
}

inline bool use_openat2() {
#if defined(DESCENT_FS_FILE_COMPILE_OPENAT2)
	Data data = wrap_openat2(AT_FDCWD, "", Access::EDIT);
	if (data.fd >= 0) {
		wrap_close(data.fd);
		return false;
	}

	if (errno != ENOENT)
		return false;

	data = wrap_openat2(AT_FDCWD, "/", Access::READ);
	if (data.fd >= 0) {
		wrap_close(data.fd);
		return false;
	}

	return errno == EFTYPE;
#else
	return false;
#endif
}

inline Data open(i32 root, const char *name, Access a) {
	static const bool USE_OPENAT2 = use_openat2();
	return (USE_OPENAT2) ? wrap_openat2(root, name, a) : wrap_openat(root, name, a);
}

}

#endif
