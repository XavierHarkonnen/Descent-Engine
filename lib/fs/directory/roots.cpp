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

#include <descent/fs.hpp>

extern "C" {
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
}

#include <descent/memory/stack.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/thread/once.hpp>
#include <descent/type/core.hpp>

#include "../name.hpp"
#include "system.hpp"

namespace descent::fs {

static thread::Mutex mutex;
static thread::atomic::Flag initialized(false);
static const Directory INVALID_DIRECTORY;
static char program_buffer[name::MAX_NAME_LENGTH + 1];
static string program_name = {};

// TODO:
// - Should not create home or base directories
// - Cache only success, so a root that failed is tried again
// - Open first, and only call mkdir on ENOENT

bool accept_fd(i32 fd) {
	return fd >= 0;
}

static string get_env(string name) {
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
	return directory::open(get_env(string("HOME")));
}

static int home_fd() {
	static thread::Once once(generate_home_fd, accept_fd, -1);
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

	return directory::open(base);
}

static int generate_root_fd(string name, string env_name, string fallback) {
	int env_fd = -1;

	string env_path = get_env(env_name);

	// "All paths set in these environment variables must be absolute.
	// If an implementation encounters a relative path in any of these
	// variables it should consider the path invalid and ignore it."
	if (env_path && env_path[0] == '/')
		env_fd = directory::open(env_path);

	if (env_fd < 0)
		env_fd = directory::open(home_fd(), fallback);

	if (env_fd < 0)
		return -1;

	int element = directory::open(env_fd, name);
	close(env_fd);

	return element;
}

bool set_program_name(string name) {
	thread::mutex::Scope scope(mutex);

	if (initialized.load(thread::atomic::Order::ACQUIRE))
		return false;

	const u64 LENGTH = name::length(name);
	if (LENGTH == 0)
		return false;

	memcpy(program_buffer, name.data(), LENGTH);
	program_buffer[LENGTH] = '\0';

	program_name = string(program_buffer, LENGTH + 1);

	initialized.set(thread::atomic::Order::RELEASE);

	return true;
}

string get_program_name() {
	thread::mutex::Scope scope(mutex);

	return program_name;
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

}
