#ifndef DESCENT_SOURCE_FILE_PATH_H
#define DESCENT_SOURCE_FILE_PATH_H

#include <descent/file.h>
#include <descent/type/core.h>
#include <descent/rcode.h>

// All paths consist of a root (base, config, data, state, or cache) and a string (max LIMIT_PATH_LENGTH)

/*
A filesystem path is a relative path from one of the engine's predefined roots.
Paths use / as the only separator.
Each path consists of 1–16 non-empty segments separated by /.
Each segment contains only printable ascii characters.
Resolved segments may not equal . or .., may not begin with ., and should not contain reserved device names.
Resolved paths are limited to 239 ASCII characters plus a terminating NUL.
*/

#define FILE_PATH_SEPARATOR '/'
#define FILE_PATH_TERMINATOR '\0'

// path.inv_length stores the inverted length of the path string as FILE_MAX_STRING_LENGTH - length.
//
// When the string is less than FILE_MAX_STRING_LENGTH bytes long, it is NUL-terminated as normal in
// the byte after the end of the string, and the inverted length byte stores the inverted length as
// normal.
//
// When the string is exactly FILE_MAX_STRING_LENGTH bytes long, the byte after the end of the string
// is the inverted length byte. Because FILE_MAX_STRING_LENGTH - FILE_MAX_STRING_LENGTH = 0, the
// inverted length byte is zero and thus also operates as the NUL terminator.
//
// As such, the path string is always a NUL-terminated string, without encoding length in an extra byte.

struct path {
	// meta 0...2: path root
	// meta 3: reserved
	// meta 4...7: number of file separators in path string
	u8 meta;
	u8 separators[FILE_PATH_MAX_SEPARATORS]; // indices of file separators in path string
	char string[FILE_PATH_MAX_STRING_LENGTH]; // NUL-terminated path string
	u8 remainder; // remaining space, also NUL terminator for full string
};

_Static_assert(sizeof(struct path) == 256, "Size of path should be 256 bytes");

u8 path_get_root(const struct path *p);

u8 path_get_separator_count(const struct path *p);

u8 path_get_string_length(const struct path *p);

rcode path_init(struct path *restrict p, u8 root, const char *restrict string);

#endif