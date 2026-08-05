#include "path.h"

#include <string.h>

#include <descent/file.h>
#include <descent/rcode.h>
#include <descent/type/core.h>

struct path {
	u8 segment_count;
	u8 segment_ends[PATH_MAX_SEGMENTS];
	char resolved_path[PATH_MAX_SIZE];
};

static inline bool path_root_is_valid(u8 root) {
	return root < FILE_ROOT_COUNT;
}

rcode resolve_path(struct path *p, const char *path) {

}

rcode file_open(u64 handle, u8 root, const char *path, u8 mode) {
	if (!path)
		return DESCENT_ERROR_NULL;

	if (!path_root_is_valid(root))
		return path_init_error(p, FILE_ERROR_PATH_ROOT);

	u8 segment_count;
	u8 segment_ends[PATH_MAX_SEGMENTS];
	char resolved_path[PATH_MAX_SIZE];

	// Resolve path
	// Create requisite directories (if FILE_MODE_READWRITE)
	// Open the file (create if FILE_MODE_READWRITE)

	if (string[0] == '\0')
		return path_init_error(p, FILE_ERROR_PATH_EMPTY);

	if (string[0] == '/')
		return path_init_error(p, FILE_ERROR_PATH_ABSOLUTE);

	u8 segment_index = 0;
	const char *segments[FILE_PATH_MAX_SEGMENTS] = {0};
	u64 segment_lengths[FILE_PATH_MAX_SEGMENTS] = {0};
	
	for (u64 i = 0; string[i] != FILE_PATH_TERMINATOR; (void)0) {
		u64 start = i;

		const char *segment = &string[i];

		while (string[i] != FILE_PATH_TERMINATOR && string[i] != FILE_PATH_SEPARATOR) {
			if (!path_char_is_valid(string[i]))
				return path_init_error(p, FILE_ERROR_PATH_CHAR);

			++i;
		}

		u64 segment_length = i - start;

		// Skip over file separators
		if (string[i] == FILE_PATH_SEPARATOR)
			++i;

		// Collapse empty segments
		if (segment_length == 0)
			continue;

		// Collapse identity segments
		if (segment_length == 1 && segment[0] == '.')
			continue;

		// Collapse parent segments
		if (segment_length == 2 && segment[0] == '.' && segment[1] == '.') {
			if (segment_index == 0)
				return path_init_error(p, FILE_ERROR_PATH_ESCAPE);

			--segment_index;
			continue;
		}

		// Commit segment to stack
		segments[segment_index] = segment;
		segment_lengths[segment_index] = segment_length;

		++segment_index;

		if (segment_index >= FILE_PATH_MAX_SEGMENTS)
			return path_init_error(p, FILE_ERROR_PATH_DEPTH);
	}

	path_set_root(p, root);
	path_set_separator_count(p, segment_index - 1);

	char *output = p->string;
	for (u64 i = 0; i < segment_index; ++i) {
		u64 grab_size = segment_lengths[i] + 1;
		u64 total_size = (u64) (output - p->string) + grab_size;

		if (total_size > FILE_PATH_MAX_STRING_SIZE)
			return path_init_error(p, FILE_ERROR_PATH_LENGTH);

		memcpy(output, segments[i], grab_size);
		output += grab_size;
	}

	output--;

	// Remove trailing separator
	if ((output > p->string) && *(output - 1) == '/')
		output--;

	u8 length = (u8) (output - p->string);

	if (length == 0)
		return path_init_error(p, FILE_ERROR_PATH_EMPTY);

	path_set_string_length(p, length);

	return DESCENT_SUCCESS;
}













static inline bool path_root_is_valid(u8 root, u8 mode) {
	switch (root) {
		case FILE_ROOT_BASE:
			return mode == 
		case FILE_ROOT_CONFIG:
		case FILE_ROOT_DATA:
		case FILE_ROOT_STATE:
		case FILE_ROOT_CACHE:
		default:
			return false;
	}
}

rcode path_validate(u8 root, const char *path, u8 mode) {
	if (!path_mode_is_valid())
	if (!path)
		return DESCENT_ERROR_NULL;

	if (!path_root_is_valid(root))
		return path_init_error(p, FILE_ERROR_PATH_ROOT);

	u8 segment_count;
	u8 segment_ends[PATH_MAX_SEGMENTS];
	char resolved_path[PATH_MAX_SIZE];

	// Resolve path
	// Create requisite directories (if FILE_MODE_READWRITE)
	// Open the file (create if FILE_MODE_READWRITE)

	if (string[0] == '\0')
		return path_init_error(p, FILE_ERROR_PATH_EMPTY);

	if (string[0] == '/')
		return path_init_error(p, FILE_ERROR_PATH_ABSOLUTE);

	u8 segment_index = 0;
	const char *segments[FILE_PATH_MAX_SEGMENTS] = {0};
	u64 segment_lengths[FILE_PATH_MAX_SEGMENTS] = {0};
	
	for (u64 i = 0; string[i] != FILE_PATH_TERMINATOR; (void)0) {
		u64 start = i;

		const char *segment = &string[i];

		while (string[i] != FILE_PATH_TERMINATOR && string[i] != FILE_PATH_SEPARATOR) {
			if (!path_char_is_valid(string[i]))
				return path_init_error(p, FILE_ERROR_PATH_CHAR);

			++i;
		}

		u64 segment_length = i - start;

		// Skip over file separators
		if (string[i] == FILE_PATH_SEPARATOR)
			++i;

		// Collapse empty segments
		if (segment_length == 0)
			continue;

		// Collapse identity segments
		if (segment_length == 1 && segment[0] == '.')
			continue;

		// Collapse parent segments
		if (segment_length == 2 && segment[0] == '.' && segment[1] == '.') {
			if (segment_index == 0)
				return path_init_error(p, FILE_ERROR_PATH_ESCAPE);

			--segment_index;
			continue;
		}

		// Commit segment to stack
		segments[segment_index] = segment;
		segment_lengths[segment_index] = segment_length;

		++segment_index;

		if (segment_index >= FILE_PATH_MAX_SEGMENTS)
			return path_init_error(p, FILE_ERROR_PATH_DEPTH);
	}

	path_set_root(p, root);
	path_set_separator_count(p, segment_index - 1);

	char *output = p->string;
	for (u64 i = 0; i < segment_index; ++i) {
		u64 grab_size = segment_lengths[i] + 1;
		u64 total_size = (u64) (output - p->string) + grab_size;

		if (total_size > FILE_PATH_MAX_STRING_SIZE)
			return path_init_error(p, FILE_ERROR_PATH_LENGTH);

		memcpy(output, segments[i], grab_size);
		output += grab_size;
	}

	output--;

	// Remove trailing separator
	if ((output > p->string) && *(output - 1) == '/')
		output--;

	u8 length = (u8) (output - p->string);

	if (length == 0)
		return path_init_error(p, FILE_ERROR_PATH_EMPTY);

	path_set_string_length(p, length);

	return DESCENT_SUCCESS;
}
