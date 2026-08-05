#define _GNU_SOURCE

#include <descent/file.h>

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include "env.h"
#include "utils.h"

#include <stdio.h>

#define FILE_HANDLE(identifier, generation) ((((u64) (generation)) << 32) | ((u64) slot))
#define FILE_HANDLE_IDENTIFIER(handle) (handle & 0xFFFFFFFFu)
#define FILE_HANDLE_GENERATION(handle) (handle >> 32)

struct file {
	i32 descriptor;
	u32 generation;
	u64 size;
	void *mapping;
	bool writeable;
};

static struct file files[FILE_MAX_HANDLES];

static bool file_root_is_valid(u8 root) {
	return root < FILE_ROOT_COUNT;
}

static bool file_mode_is_valid(u8 root, bool writable) {
	return (root == FILE_ROOT_BASE) ? !writable : true;
}

// Paths must:
// - be relative
// - contain only printable ASCII characters
// - not contain symlinks
// - be canonical (no duplicate file separators "//", no self directories ".", no parent directories "..")
static rcode file_validate_path(const char *path) {
	if (!path)
		return DESCENT_ERROR_NULL;

	if (*path == '\0')
		return FILE_ERROR_PATH_EMPTY;

	if (*path == '/')
		return FILE_ERROR_PATH_ABSOLUTE;

	for (char c = *path; c; c = *++path) {
		if (c < ' ' || c > '~')
			return FILE_ERROR_PATH_CHAR;
	}

	return DESCENT_SUCCESS;
}

rcode file_open(u64 *handle, u8 root, const char *path, bool writable) {
	if (!handle)
		return DESCENT_ERROR_NULL;

	if (!file_root_is_valid(root))
		return FILE_ERROR_ROOT;

	if (!file_mode_is_valid(root, writable))
		return FILE_ERROR_MODE;

	rcode result = file_validate_path(path);
	if (!RCODE_SUCCEEDED(result))
		return result;

	/* ... */

	return DESCENT_SUCCESS;
}

// END IMPLEMENTATION BRAINSTORM



// Always Open Files:
// - stdin
// - stdout
// - stderr
// - FILE_ROOT_BASE
// - FILE_ROOT_CONFIG
// - FILE_ROOT_DATA
// - FILE_ROOT_STATE
// - FILE_ROOT_CACHE

/*
A filesystem path is a relative path from one of the engine's predefined roots.
Paths use / as the only separator.
Each path consists of 1–16 non-empty segments separated by /.
Each segment contains only `A`–`Z`, `a`–`z`, `0`–`9`, `_`, `-`, and `.`.
Resolved segments may not equal . or .., may not begin with ., and should not contain reserved device names.
Resolved paths are limited to 239 ASCII characters plus a terminating NUL.
A resolved path may not contain any symbolic links.
At most 64 files may be open simultaneously.
*/

// Centralize filewise descriptor ownership here - sockets and files together.
// Socket descriptors are separate

// Keep table of open files

// Descriptor Paging
// If a file can't be opened because too many descriptors are open, page the oldest accessed one out
// Always open files and sockets cannot be paged out

#define MAX_FILES   256
#define MAX_SOCKETS 32


typedef union {
	u64 handle;
	struct {
		u8 generation;
		u8 identifier;
		u16 _reserved;
	};
} handle;

struct descriptor {
	int descriptor;
	u8 generation;
	bool active;
	bool pinned; // Pinned files cannot be evicted
	u64 last_access; // time of last access in nanoseconds
};

struct file {
	struct descriptor descriptor;
	struct path path;

	u64 offset;
	u32 flags;
};

struct address {
	u128 ip_address;
	u16 port;
	u16 protocol;
};

struct socket {
	struct descriptor descriptor;
	struct address address;
};

static struct file files[MAX_FILES];
static struct socket sockets[MAX_SOCKETS];

void *open_file(u8 root, const char *path) {
	char buffer[PATH_SIZE_MAX];

	if (!resolve_path(path, buffer))
		return NULL;

	int root_fd;
	switch (root) {
		case FILE_ROOT_BASE:   root_fd = file_world->base;   break;
		case FILE_ROOT_CONFIG: root_fd = file_world->config; break;
		case FILE_ROOT_DATA:   root_fd = file_world->data;   break;
		case FILE_ROOT_STATE:  root_fd = file_world->state;  break;
		case FILE_ROOT_CACHE:  root_fd = file_world->cache;  break;
		default: return NULL;
	}

	if (!make_dirs(root_fd, buffer))
		return NULL;

	static const struct open_how open_how = {
		.flags = O_PATH | O_CREAT | O_CLOEXEC,
		.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS | RESOLVE_NO_MAGICLINKS | RESOLVE_NO_XDEV
	};

	int file_fd = openat2(root_fd, buffer, &open_how, sizeof(open_how));

	(void) file_fd;

	return NULL;
}
