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

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <linux/openat2.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
}

#include <descent/memory/stack.hpp>

#include "file.hpp"
#include "utils.hpp"

#if defined(SYS_openat2) && defined(OPENAT2_REGULAR) && defined(EFTYPE)
#define DESCENT_FILE_COMPILE_OPENAT2
#endif

namespace descent {

static constexpr int FILE_FLAGS = O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW;
static constexpr int FILE_MODE = 0600;

static int wrap_file_openat2(int root, const char *name, int flags, int mode) {
#if defined(DESCENT_FILE_COMPILE_OPENAT2)
	struct open_how how = {};
	how.flags = static_cast<u64>(flags) | OPENAT2_REGULAR;
	how.mode = static_cast<u64>(mode);

	long fd;
	do {
		fd = syscall(SYS_openat2, root, name, &how, sizeof(how));
	} while (fd < 0 && errno == EINTR);

	return static_cast<int>(fd);
#else
	(void) root;
	(void) name;
	(void) flags;
	(void) mode;

	return -1;
#endif
}

static int wrap_file_openat(int root, const char *name, int flags, int mode) {
	int fd;
	do {
		fd = openat(root, name, flags | O_NONBLOCK | O_NOCTTY, mode);
	} while (fd < 0 && errno == EINTR);

	return fd;
}

static bool use_openat2_regular() {
#if defined(DESCENT_FILE_COMPILE_OPENAT2)
	int fd = wrap_file_openat2(AT_FDCWD, "/", O_RDONLY | O_CLOEXEC | O_NOFOLLOW, 0);
	if (fd >= 0) {
		close(fd);
		return false;
	}
	return errno == EFTYPE;
#else
	return false;
#endif
}

int open_file(int root, const char *name) {
	static const bool USE_OPENAT2_REGULAR = use_openat2_regular();

	// Kernel 7.2
	// Attempt to use OPENAT2_REGULAR to open and check the file
	if (USE_OPENAT2_REGULAR)
		return wrap_file_openat2(root, name, FILE_FLAGS, FILE_MODE);

	// Open, then verify the resulting descriptor.
	int fd = wrap_file_openat(root, name, FILE_FLAGS, FILE_MODE);
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

File::File(const Directory &parent, string name) {
	if (!validate_path(parent, name))
		return;

	int parent_fd = 

	if (name.c_safe()) {
		fd = open_file(parent.ha, name.data());
		return;
	}

	auto buffer = memory::stack::Allocator::copy(name);
	if (!buffer)
		return;

	fd = open_file(parent.fd, buffer.data());
}

}
