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

#ifndef DESCENT_LIB_FS_FILE_SYSTEM_HPP
#define DESCENT_LIB_FS_FILE_SYSTEM_HPP

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

#include <descent/fs.hpp>
#include <descent/type/core.hpp>
#include <descent/memory/stack.hpp>

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

// Kernel 7.2
inline i32 wrap_openat2(i32 root, const char *name, Access a) {
#if defined(DESCENT_FS_FILE_COMPILE_OPENAT2)
	struct open_how how = {};
	
	how.flags = static_cast<u64>(flags(a)) | OPENAT2_REGULAR;
	how.mode  = static_cast<u64>(mode(a));

	long fd;
	do {
		fd = ::syscall(SYS_openat2, root, name, &how, sizeof(how));
	} while (fd < 0 && errno == EINTR);

	return static_cast<i32>(fd);
#else
	(void) root;
	(void) name;
	(void) a;

	errno = ENOSYS;

	return -1;
#endif
}

inline i32 wrap_openat(i32 root, const char *name, Access a) {
	i32 fd;
	do {
		fd = ::openat(root, name, flags(a) | O_NONBLOCK | O_NOCTTY, mode(a));
	} while (fd < 0 && errno == EINTR);

	return fd;
}

inline bool use_openat2() {
#if defined(DESCENT_FS_FILE_COMPILE_OPENAT2)
	i32 fd = wrap_openat2(AT_FDCWD, "", Access::EDIT);
	if (fd >= 0) {
		close(fd);
		return false;
	}

	if (errno != ENOENT)
		return false;

	fd = wrap_openat2(AT_FDCWD, "/", Access::READ);
	if (fd >= 0) {
		close(fd);
		return false;
	}

	return errno == EFTYPE;
#else
	return false;
#endif
}

inline i32 open(i32 root, const char *name, Access a) {
	static const bool USE_OPENAT2 = use_openat2();

	// Open and verify descriptor in one syscall
	if (USE_OPENAT2)
		return wrap_openat2(root, name, a);

	// Open, then verify the resulting descriptor.
	i32 fd = wrap_openat(root, name, a);
	if (fd < 0)
		return -1;

	struct stat st;
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode)) {
		close(fd);
		return -1;
	}

	if (fcntl(fd, F_SETFL, 0) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

}

#endif
