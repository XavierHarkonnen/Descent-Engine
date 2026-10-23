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

static int open_dir(string path) {
	if (!path || path[0] != '/')
		return -1;

	if (path.c_safe())
		return open(path.data(), O_PATH | O_DIRECTORY | O_CLOEXEC);

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return -1;

	return open(buffer.data(), O_PATH | O_DIRECTORY | O_CLOEXEC);
}

static int open_dir(int root, string path) {
	if (root < 0 || !path || !path[0] || path[0] == '/')
		return -1;

	if (path.c_safe())
		return openat(root, path.data(), O_PATH | O_DIRECTORY | O_CLOEXEC);

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return -1;

	return openat(root, buffer.data(), O_PATH | O_DIRECTORY | O_CLOEXEC);
}


static bool make_dir_cstr(const char *path) {
	if (mkdir(path, DIR_MODE) == 0)
		return true;

	if (errno != EEXIST)
		return false;

	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static bool make_dir_cstr(int root, const char *path) {
	if (mkdirat(root, path, DIR_MODE) == 0)
		return true;

	if (errno != EEXIST)
		return false;

	struct stat st;
	return fstatat(root, path, &st, 0) == 0 && S_ISDIR(st.st_mode);
}


static bool make_dir(string path) {
	if (!path || path[0] != '/')
		return false;

	if (path.c_safe())
		return make_dir_cstr(path.data());

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return false;

	return make_dir_cstr(buffer.data());
}

static bool make_dir(int root, string path) {
	if (root < 0 || !path || !path[0] || path[0] == '/')
		return false;

	if (path.c_safe())
		return make_dir_cstr(root, path.data());

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return false;

	return make_dir_cstr(root, buffer.data());
}


static bool make_dirs(string path) {
	if (!path || path[0] != '/')
		return false;

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return false;

	for (u64 i = 1; i < buffer.capacity(); ++i) {
		if (buffer[i] == '\0') {
			if (buffer[i - 1] != '/')
				return make_dir_cstr(buffer.data());

			return true;
		}

		if (buffer[i] == '/' && buffer[i - 1] != '/') {
			buffer[i] = '\0';
		
			if (!make_dir_cstr(buffer.data()))
				return false;

			buffer[i] = '/';
		}
	}

	__builtin_unreachable();
}

static bool make_dirs(int root, string path) {
	if (root < 0 || !path || !path[0] || path[0] == '/')
		return false;

	auto buffer = memory::stack::Allocator::copy(path);
	if (!buffer)
		return false;

	for (u64 i = 1; i < buffer.capacity(); ++i) {
		if (buffer[i] == '\0') {
			if (buffer[i - 1] != '/')
				return make_dir_cstr(root, buffer.data());

			return true;
		}

		if (buffer[i] == '/' && buffer[i - 1] != '/') {
			buffer[i] = '\0';
		
			if (!make_dir_cstr(root, buffer.data()))
				return false;

			buffer[i] = '/';
		}
	}

	__builtin_unreachable();
}


static int make_open_dir(string path) {
	make_dirs(path);
	// This operation is TOCTOU, so open_dir is the canonical check of existence
	return open_dir(path);
}

static int make_open_dir(int root, string path) {
	make_dirs(root, path);
	// This operation is TOCTOU, so open_dir is the canonical check of existence
	return open_dir(root, path);
}


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

	u64 index = (slash == 0) ? 1 : slash;
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
	const u64 LENGTH = (C_LENGTH == string::INVALID) ? name.size() : C_LENGTH;

	if (LENGTH >= MAX_PROGRAM_NAME_SIZE)
		return false;

	memcpy(program_buffer, name.data(), LENGTH);
	program_buffer[LENGTH] = '\0';

	program_name = string(program_buffer, LENGTH + 1);

	initialized.set(thread::atomic::Order::RELEASE);

	return true;
}

Directory::Directory(const Directory &parent, string name) {
	if (!parent || !validate_name(name))
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

		fd = other.fd;
		other.fd = -1;
	}

	return *this;
}

Directory::~Directory() {
	if (fd >= 0)
		close(fd);
}

}
