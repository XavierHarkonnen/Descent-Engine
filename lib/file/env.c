#define _GNU_SOURCE

#include "env.h"

#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <descent/sys/core.h>

#include "utils.h"


static struct file_roots roots;

const struct file_roots *file_get_roots(void) {
	return &roots;
}


static i32 roots_get_element(const char *name, const char *env_name, i32 home, const char *fallback) {
	i32 env_fd = -1;

	const char *env_path = getenv(env_name);

	// "All paths set in these environment variables must be absolute.
	// If an implementation encounters a relative path in any of these
	// variables it should consider the path invalid and ignore it."
	if (env_path && env_path[0] == '/')
		env_fd = open_raw_dir(env_path);
	
	if (env_fd < 0)
		env_fd = open_dir(home, fallback);

	if (env_fd < 0)
		return -1;

	i32 element = open_dir(env_fd, name);
	close(env_fd);

	return element;
}


__attribute__((constructor))
static void roots_construct_base(void) {
	char base[PATH_MAX];
	ssize_t base_size = readlink("/proc/self/exe", base, sizeof(base));

	if (base_size < 0)
		sys_fatal("Could not discover executable");

	if (base_size == sizeof(base))
		sys_fatal("Executable path too long");

	base[base_size] = '\0';

	if (base[0] != '/')
		sys_fatal("Executable path cannot be relative path");

	char *slash = strrchr(base, '/');
	if (slash == base)
		slash[1] = '\0';
	else
		slash[0] = '\0';

	roots.base = open(base, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (roots.base < 0)
		sys_fatal("Could not open executable base directory");
}


__attribute__((constructor))
static void roots_construct_elements(void) {
	const char *home_path = getenv("HOME");
	if (!home_path)
		sys_fatal("Could not discover home path");

	if (home_path[0] != '/')
		sys_fatal("Home path cannot be relative path");

	i32 home = open(home_path, O_PATH | O_DIRECTORY | O_CLOEXEC);
	if (home < 0)
		sys_fatal("Could not open home directory");

	roots.config = roots_get_element("granbarrow", "XDG_CONFIG_HOME", home, ".config");
	if (roots.config < 0)
		sys_fatal("Could not open config directory");

	roots.data = roots_get_element("granbarrow", "XDG_DATA_HOME", home, ".local/share");
	if (roots.data < 0)
		sys_fatal("Could not open data directory");

	roots.state = roots_get_element("granbarrow", "XDG_STATE_HOME", home, ".local/state");
	if (roots.state < 0)
		sys_fatal("Could not open state directory");

	roots.cache = roots_get_element("granbarrow", "XDG_CACHE_HOME", home, ".cache");
	if (roots.cache < 0)
		sys_fatal("Could not open cache directory");

	close(home);
}


__attribute__((destructor))
static void roots_destroy(void) {
	close(roots.base);
	close(roots.config);
	close(roots.data);
	close(roots.state);
	close(roots.cache);
}
