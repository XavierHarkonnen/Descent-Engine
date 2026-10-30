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

#include "fs.hpp"

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

#include <descent/memory/stack.hpp>

#include "utils.hpp"

#if defined(SYS_openat2) && defined(OPENAT2_REGULAR) && defined(EFTYPE)
#define DESCENT_FILE_COMPILE_OPENAT2
#endif

namespace descent::fs {

static_assert(sizeof(int) == sizeof(i32), "File descriptors are stored as i32");

constexpr i32 file_flags(Access a) {
	if (a == Access::EDIT)
		return O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW;

	return O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
}

constexpr i32 file_mode(Access a) {
	return a == Access::EDIT ? 0600 : 0;
}

static i32 wrap_file_openat2(i32 root, const char *name, Access a) {
#if defined(DESCENT_FILE_COMPILE_OPENAT2)
	struct open_how how = {};
	how.flags = static_cast<u64>(file_flags(a)) | OPENAT2_REGULAR;
	how.mode = static_cast<u64>(file_mode(a));

	long fd;
	do {
		fd = syscall(SYS_openat2, root, name, &how, sizeof(how));
	} while (fd < 0 && errno == EINTR);

	return static_cast<i32>(fd);
#else
	(void) root;
	(void) name;
	(void) a;

	errno = ENOSYS

	return -1;
#endif
}

static i32 wrap_file_openat(i32 root, const char *name, Access a) {
	i32 fd;
	do {
		fd = openat(root, name, file_flags(a) | O_NONBLOCK | O_NOCTTY, file_mode(a));
	} while (fd < 0 && errno == EINTR);

	return fd;
}

// Kernel 7.2
static bool use_openat2_regular() {
#if defined(DESCENT_FILE_COMPILE_OPENAT2)
	i32 fd = wrap_file_openat2(AT_FDCWD, "", Access::EDIT);
	if (fd >= 0) {
		close(fd);
		return false;
	}

	if (errno != ENOENT)
		return false;

	fd = wrap_file_openat2(AT_FDCWD, "/", Access::READ);
	if (fd >= 0) {
		close(fd);
		return false;
	}

	return errno == EFTYPE;
#else
	return false;
#endif
}

static i32 open_file(i32 root, const char *name, Access a) {
	static const bool USE_OPENAT2_REGULAR = use_openat2_regular();

	// Open and verify descriptor in one syscall
	if (USE_OPENAT2_REGULAR)
		return wrap_file_openat2(root, name, a);

	// Open, then verify the resulting descriptor.
	i32 fd = wrap_file_openat(root, name, a);
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

File::File(const Directory &parent, string name, Access a) {
	if (!validate_path(parent, name))
		return;

	if (name.c_safe()) {
		h = Handle(open_file(parent.handle().h, name.data(), a));
		return;
	}

	auto buffer = memory::stack::Allocator::copy(name);
	if (!buffer)
		return;

	h = Handle(open_file(parent.handle().h, buffer.data(), a));
}

}
