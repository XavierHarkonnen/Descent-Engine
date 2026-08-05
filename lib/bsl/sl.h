#ifndef DESCENT_BSL_H
#define DESCENT_BSL_H

#include <descent/type/core.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <unistd.h>

// Binary Structured Logging

/* BSL metadata
+======================+
| metadata header      |
+======================+
| site table           |
+----------------------+
| site 0               |
| site 1               |
| site 2               |
| ...                  |
+======================+
| file pool            |
+----------------------+
| file 0               |
| file 1               |
| file 2               |
| ...                  |
+======================+
| message pool         |
+----------------------+
| message 0            |
| message 1            |
| message 2            |
| ...                  |
+======================+
*/

/* BSL segment
+======================+
| segment header       |
+======================+
| entry table          |
+----------------------+
| entry 0              |
| entry 1              |
| entry 2              |
| ... (grows upward)   |
+~~~~~~~~~~~~~~~~~~~~~~+
|      free space      |
+~~~~~~~~~~~~~~~~~~~~~~+
| ... (grows downward) |
| arg blob 2           |
| arg blob 1           |
| arg blob 0           |
+----------------------+
| argument pool        |
+======================+
*/

// mmap segments directly to disk
// logs/session-0000
// - meta
// - segment-0000.log
// - segment-0001.log
// - segment-0002.log
// - ...

// segments are immutable after rotation; external tools can operate on them without interfering with the writer

#define BSL_SIGNATURE  0x454C494620534C42ull /* "BSL FILE" */
#define BSL_VERSION(major, minor, patch, variant) ((((u32)(variant)) << 29u) | (((u32)(major)) << 22u) | (((u32)(minor)) << 12u) | ((u32)(patch)))
#define BSL_VERSION_METADATA 1
#define BSL_VERSION_SEGMENT 1

#define BSL_SIZE_METADATA  (4 * 1024)   /* Maximum size of a metadata file: 4 kilobytes */
#define BSL_SIZE_SEGMENT   (256 * 1024) /* Maximum size of a segment file: 256 kilobytes */
#define BSL_MAX_SIZE_ARGUMENTS (1024)   /* Maximum size of an argument blob: 1 kilobyte */

#define BSL_INVALID_SITE  U32_MAX /* Invalid site reference */
#define BSL_INVALID_FRAME U64_MAX /* Invalid frame (before/after frame counting begins) */

enum {
	BSL_FILE_TYPE_METADATA = 1,
	BSL_FILE_TYPE_SEGMENT  = 2,
};

enum {
	BSL_LEVEL_TRACE,
	BSL_LEVEL_DEBUG,
	BSL_LEVEL_INFO,
	BSL_LEVEL_WARN,
	BSL_LEVEL_ERROR,
	BSL_LEVEL_FATAL
};

struct bsl_segment {
	struct bsl_header header;

	u64 timestamp_first;
	u64 timestamp_last;

	u64 entry_count;

	u32 entry_begin;      /* sizeof(struct bsl_segment) */
	u32 entry_end;        /* next entry append (grows up) */

	u32 arguments_begin;  /* file size */
	u32 arguments_end;    /* next argument allocation (grows down) */
};

struct bsl_site {
	u32  offset_file;
	u32  offset_message;
	u32  line;
	u8   module;
	u8   level;
	u16  size_arguments;
};

struct bsl_entry {
	u64 timestamp; // monotonic nanoseconds
	u64 frame;     // event frame
	u32 thread;    // event thread
	u32 site;      // log site index
	u32 offset_arguments;
};

_Static_assert(sizeof(struct bsl_segment) % _Alignof(struct bsl_entry) == 0, "entry alignment");

static void *bsl_create_file(const char *path, u32 size) {
	int fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0644);
	if (fd < 0)
		return NULL;

	if (ftruncate(fd, size) < 0) {
		close(fd);
		unlink(path);
		return NULL;
	}

	void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (map == MAP_FAILED) {
		unlink(path);
		return NULL;
	}

	return map;
}

static struct bsl_metadata *bsl_create_metadata(const char *path) {
	struct bsl_metadata *metadata = bsl_create_file(path, BSL_SIZE_METADATA);
	if (!metadata)
		return NULL;

	metadata->header.signature = BSL_SIGNATURE;
	metadata->header.type      = BSL_FILE_TYPE_METADATA;
	metadata->header.version   = BSL_VERSION_METADATA;
	metadata->header.flags     = 0;
	metadata->header.size      = BSL_SIZE_METADATA;

	return metadata;
}

static struct bsl_segment *bsl_create_segment(const char *path) {
	struct bsl_segment *segment = bsl_create_file(path, BSL_SIZE_SEGMENT);
	if (!segment)
		return NULL;

	segment->header.signature = BSL_SIGNATURE;
	segment->header.type      = BSL_FILE_TYPE_SEGMENT;
	segment->header.version   = BSL_VERSION_SEGMENT;
	segment->header.flags     = 0;
	segment->header.size      = BSL_SIZE_SEGMENT;
	segment->timestamp_first  = 0;
	segment->timestamp_last   = 0;
	segment->entry_count      = 0;
	segment->entry_begin      = sizeof(struct bsl_segment);
	segment->entry_end        = sizeof(struct bsl_segment);
	segment->arguments_begin  = BSL_SIZE_SEGMENT;
	segment->arguments_end    = BSL_SIZE_SEGMENT;

	return segment;
}

void bsl_emit(struct bsl_metadata *metadata, struct bsl_segment *segment, const struct bsl_site_data *site, ...) {
	if (!metadata || !segment || !site) return;
	// Translate site data to site index

	struct bsl_segment *header = segment;
	// If the segment is too full, close it out and create a new one

	site->_message
	(void) site;
	// TODO
}

// Message string formatting:
// - %%: literal "%"
// - %s: string
// - %c: character
// - %b: boolean
// - %i*: signed integer width (8, 16, 32, 64, 128)
// - %u*: unsigned integer width (8, 16, 32, 64, 128)
// - %f*: floating point width (32, 64)
#define BSL_LOG(level, module, message, ...) do {                                            \
	static const struct bsl_site_data __attribute__((section(".bsl"), used)) bsl_site_data = { \
		._file    = __FILE__,                                                                    \
		._message = message,                                                                     \
		._line    = __LINE__,                                                                    \
		._module  = module,                                                                      \
		._level   = level                                                                        \
	};                                                                                         \
	bsl_emit(&bsl_site_data, __VA_OPT__(,) __VA_ARGS__);                                       \
} while (0)

u32 bsl_site_to_index(const struct bsl_site *site) {
	return (u32) (site - bsl_sites);
}

void bsl_test(void) {
	BSL_LOG(BSL_LEVEL_WARN, 0, "Hello, world!");
}

#endif

/*
"
I do not like structured logs, because they convert logging from a necessary evil into a deliverable.
It is VERY frustrating to have a problem with a system, and when you look at the log files for hints,
you find you're logging many megabytes per hour, because over the years people keep adding log lines
"Because maybe they'll be helpful", and nobody ever removes them for Chesterton's Fence reasons. If
possible log lines should come with an expiration date and a tracking bug, and the goal should be to
get rid of the log lines, not to turn them into a defined output of your system.
"
*/
