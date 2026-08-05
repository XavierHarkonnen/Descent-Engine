#define _GNU_SOURCE

#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <linux/openat2.h>
#include <string.h>
#include <sys/stat.h>

#include <descent/file.h>
#include <descent/type/core.h>

#define FILE_MODE_DEFAULT 0700

bool make_raw_dir(const char *path) {
	if (mkdir(path, FILE_MODE_DEFAULT) == 0)
		return true;

	if (errno != EEXIST)
		return false;

	struct stat st;

	if (lstat(path, &st) != 0)
		return false;

	return S_ISDIR(st.st_mode);
}


bool make_raw_dirs(const char *path) {
	char path_buffer[PATH_MAX];

	if (strlcpy(path_buffer, path, sizeof(path_buffer)) >= sizeof(path_buffer))
		return false;

	char *separator = path_buffer;
	while ((separator = strchr(separator + 1, '/'))) {
		*separator = '\0';

		if (!make_raw_dir(path_buffer))
			return false;

		*separator = '/';
	}

	return make_raw_dir(path_buffer);
}


int open_raw_dir(const char *path) {
	if (!make_raw_dirs(path))
		return -1;

	return open(path, O_PATH | O_DIRECTORY | O_CLOEXEC);
}


bool make_dir(int root, const char *path) {
	if (mkdirat(root, path, FILE_MODE_DEFAULT) == 0)
		return true;

	if (errno != EEXIST)
		return false;

	struct stat st;

	if (fstatat(root, path, &st, AT_SYMLINK_NOFOLLOW) != 0)
		return false;

	return S_ISDIR(st.st_mode);
}


bool make_dirs(int root, const char *path) {
	char path_buffer[PATH_MAX];

	if (strlcpy(path_buffer, path, sizeof(path_buffer)) >= sizeof(path_buffer))
		return false;

	char *separator = path_buffer;
	while ((separator = strchr(separator + 1, '/'))) {
		*separator = '\0';

		if (!make_dir(root, path_buffer))
			return false;

		*separator = '/';
	}

	return make_dir(root, path_buffer);
}


int open_dir(int root, const char *path) {
	if (!make_dirs(root, path))
		return -1;

	static const struct open_how open_how = {
		.flags = O_PATH | O_DIRECTORY | O_CLOEXEC,
		.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS
	};

	return openat2(root, path, &open_how, sizeof(open_how));
}
