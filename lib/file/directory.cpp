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

#include "file.hpp"

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <descent/type/string.hpp>
#include <descent/thread/once.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/memory/stack.hpp>

#include "state.hpp"
#include "utils.hpp"

namespace descent {

static constexpr int DIR_MODE = 0700;

// make_dir functions are all best-effort, not promises of existence
// open_dir is the only canonical proof of existence and type
// TODO: Minimize the number of system calls in the happy path and overall as possible
// Maybe we can assume that the directory probably exists

static int wrap_dir_open(const char *name) {
	static constexpr int DIR_FLAGS_REL = O_PATH | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;

	int fd;
	do {
		fd = open(name, DIR_FLAGS_REL, DIR_MODE);
	} while (fd < 0 && errno == EINTR);

	return fd;
}

static int wrap_dir_openat(int root, const char *name) {
	static constexpr int DIR_FLAGS_ABS = O_PATH | O_DIRECTORY | O_CLOEXEC;

	int fd;
	do {
		fd = openat(root, name, DIR_FLAGS_ABS, DIR_MODE);
	} while (fd < 0 && errno == EINTR);

	return fd;
}

static void wrap_dir_make(const char *path) {
	int r;
	do {
		r = mkdir(path, DIR_MODE);
	} while (r < 0 && errno == EINTR);
}

static void wrap_dir_make(int root, const char *path) {
	int r;
	do {
		r = mkdirat(root, path, DIR_MODE);
	} while (r < 0 && errno == EINTR);
}

static void make_dir(char *path) {
	if (path[0] != '/')
		return;

	for (u64 i = 1; ; ++i) {
		if (path[i] == '\0') {
			if (path[i - 1] != '/')
				wrap_dir_make(path);

			return;
		}

		if (path[i] == '/' && path[i - 1] != '/') {
			path[i] = '\0';
			wrap_dir_make(path);
			path[i] = '/';
		}
	}
}

static void make_dir(int root, char *path) {
	if (path[0] == '\0' || path[0] == '/')
		return;

	for (u64 i = 1; ; ++i) {
		if (path[i] == '\0') {
			if (path[i - 1] != '/')
				wrap_dir_make(root, path);

			return;
		}

		if (path[i] == '/' && path[i - 1] != '/') {
			path[i] = '\0';
			wrap_dir_make(root, path);
			path[i] = '/';
		}
	}
}

static int open_dir(string path) {
	if (path.c_safe())
		return open(path.data(), DIR_FLAGS_ABS);

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return -1;

	return open(buffer.data(), DIR_FLAGS_ABS);
}

static int open_dir(int root, string path) {
	if (path.c_safe())
		return openat(root, path.data(), DIR_FLAGS_REL);

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return -1;

	return openat(root, buffer.data(), DIR_FLAGS_REL);
}









static int make_open_dir(string path) {
	make_dir(path);
	// This operation is TOCTOU, so open_dir is the canonical check of existence
	return open_dir(path);
}

static int make_open_dir(int root, string path) {
	make_dir(root, path);
	// This operation is TOCTOU, so open_dir is the canonical check of existence
	return open_dir(root, path);
}

// Returns reference into released memory
string get_env(string name) {
	if (!name)
		return {};

	if (name.c_safe())
		return string(getenv(name.data()));

	auto buffer = memory::stack::Allocator::copy(name);
	if (!buffer)
		return {};

	return string(getenv(buffer.data()));
}


static int generate_home_fd() {
	return open_dir(get_env(string("HOME")));
}

static int home_fd() {
	static thread::Once once(generate_home_fd);
	return once();
}


static int generate_base_fd(void) {
	char buffer[PATH_MAX];
	i64 size = readlink("/proc/self/exe", buffer, sizeof(buffer));

	if (size <= 0 || u64(size) >= sizeof(buffer))
		return -1;

	string executable(buffer, static_cast<u64>(size));
	if (executable[0] != '/')
		return -1;

	u64 slash = executable.rfind('/');
	if (slash == string::INVALID)
		return -1;

	u64 index   = (slash == 0) ? 1 : slash;
	string base = executable.sub(0, index);

	return open_dir(base);
}

static int generate_root_fd(string name, string env_name, string fallback) {
	int env_fd = -1;

	string env_path = get_env(env_name);

	// "All paths set in these environment variables must be absolute.
	// If an implementation encounters a relative path in any of these
	// variables it should consider the path invalid and ignore it."
	if (env_path && env_path[0] == '/')
		env_fd = make_open_dir(env_path);

	if (env_fd < 0)
		env_fd = make_open_dir(home_fd(), fallback);

	if (env_fd < 0)
		return -1;

	int element = make_open_dir(env_fd, name);
	close(env_fd);

	return element;
}


bool set_program_name(string name) {
	thread::mutex::Scope scope(mutex);

	if (initialized.load(thread::atomic::Order::ACQUIRE))
		return false;

	if (!validate_name(name))
		return false;

	const u64 C_LENGTH = name.c_length();
	const u64 LENGTH   = (C_LENGTH == string::INVALID) ? name.size() : C_LENGTH;

	if (LENGTH >= MAX_PROGRAM_NAME_SIZE)
		return false;

	memcpy(program_buffer, name.data(), LENGTH);
	program_buffer[LENGTH] = '\0';

	program_name = string(program_buffer, LENGTH + 1);

	initialized.set(thread::atomic::Order::RELEASE);

	return true;
}

Directory::Directory(const Directory &parent, string name) {
	if (!validate_path(parent, name))
		return;

	fd = make_open_dir(parent.fd, name);
}

const Directory &Directory::base() {
	static const Directory result(generate_base_fd());
	return result;
}

const Directory &Directory::config() {
	if (!initialized.load(thread::atomic::Order::ACQUIRE))
		return INVALID_DIRECTORY;

	static const Directory result(generate_root_fd(program_name, string("XDG_CONFIG_HOME"), string(".config")));
	return result;
}

const Directory &Directory::data() {
	if (!initialized.load(thread::atomic::Order::ACQUIRE))
		return INVALID_DIRECTORY;

	static const Directory result(generate_root_fd(program_name, string("XDG_DATA_HOME"), string(".local/share")));
	return result;
}

const Directory &Directory::state() {
	if (!initialized.load(thread::atomic::Order::ACQUIRE))
		return INVALID_DIRECTORY;

	static const Directory result(generate_root_fd(program_name, string("XDG_STATE_HOME"), string(".local/state")));
	return result;
}

const Directory &Directory::cache() {
	if (!initialized.load(thread::atomic::Order::ACQUIRE))
		return INVALID_DIRECTORY;

	static const Directory result(generate_root_fd(program_name, string("XDG_CACHE_HOME"), string(".cache")));
	return result;
}

Directory &Directory::operator =(Directory &&other) {
	if (this != &other) {
		if (fd >= 0)
			close(fd);

		fd       = other.fd;
		other.fd = -1;
	}

	return *this;
}

Directory::~Directory() {
	if (fd >= 0)
		close(fd);
}

}
