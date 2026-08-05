#ifndef DESCENT_FILE_H
#define DESCENT_FILE_H

#include <descent/type/core.h>
#include <descent/rcode.h>

#define FILE_MAX_HANDLES 256

// Contract

// All functions in this API are thread-safe.

// Temporary files are automatically deleted when closed.

// Filenames are deduplicated to indices.
//   Any number of read-only handles can be created from a file.
//   Only one writing handle can be created from a file.
//   If an existing handle is read-only and a writable handle is requested, FILE_ERROR_OPEN is returned.

// Only one mapping can be assigned to a file at once.
//   If an existing mapped handle is re-mapped, a pointer to the same mapping is given.

// Mapped files cannot be resized.
//   If an existing mapped file is resized, FILE_ERROR_MAPPED is returned.

// Writing beyond the end of a file extends it to the necessary length.

enum {
	FILE_ROOT_BASE   = 0, // Directory containing the program's binary.                     Available Permissions: Read
	FILE_ROOT_CONFIG = 1, // Directory containing the program's configuration data.         Available Permissions: Read, Write
	FILE_ROOT_DATA   = 2, // Directory containing the program's persistent data.            Available Permissions: Read, Write
	FILE_ROOT_STATE  = 3, // Directory containing the program's runtime state.              Available Permissions: Read, Write, Delete
	FILE_ROOT_CACHE  = 4, // Directory containing the program's generated, disposable data. Available Permissions: Read, Write, Delete
	FILE_ROOT_COUNT
};

// Opens a named file, creating it and all necessary directories if they do not exist
// If writeable == false, opens in read-only mode
// If writeable == true, opens in read-write mode
rcode file_open(u64 *handle, u8 root, const char *path, bool writable);

// Opens a nameless temporary file
rcode file_temp(u64 *handle);

// Closes an open file handle
rcode file_close(u64 handle);

// Deletes a file
rcode file_delete(u8 root, const char *path);

// Gets the size of a file
rcode file_size(u64 *size, u64 handle);

// Resizes a file
rcode file_resize(u64 handle, u64 size);

// Read `size` bytes from `offset` in a file
rcode file_read(u64 handle, u64 offset, void *buffer, u64 size);

// Write `size` bytes to `offset` in a file
rcode file_write(u64 handle, u64 offset, const void *buffer, u64 size);

// Maps a file to memory
rcode file_map(void **map, u64 *size, u64 handle);

// Unmaps a file from memory
rcode file_unmap(u64 handle);

#endif