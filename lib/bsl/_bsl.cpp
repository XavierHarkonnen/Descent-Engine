#include "bsl.hpp"

// Binary Structured Logging
// Intention: Make the absolute fastest possible useful logging, even if it hurts immediate readability
// - Logging is thread-local, from infrastructure to sinks.
// - At startup, the logger emits a symbol file for all metadata required to interpret the coming logs
// - Logging writes the absolute minimal amount of information into logs at runtime
// - Each thread writes to its own file (async or mmap).
// - After (or even during) logging, a separate program can be used to render the logs into plaintext.
// - Log levels can be disabled at compile and runtime.
// - Source indices can be derived from their position within a section at symbol generation time and invocation (ELF linker produces the authoritative source table, which the startup table serializes).
// - Byte ordering is platform-native, specified in the header


#include <descent/type/core.hpp>

namespace descent::bsl {

// Shared:

enum class Endian : bool {
	LITTLE = false,
	BIG    = true,
};

struct Version {
	u16 major;
	u16 minor;
	u16 patch;
	u16 variant;
};

struct Header {
	// Endian-independent portion
	u8 magic[7]; // Depends on file type
	u8 flags; // endianness (bit 0) and other flags
	u8 identifier[16]; // Uniquely identifies log session
	// Endian-dependent portion
	Version version;
};

// The identifier and version of a symbol file and its associated log files must match

// Symbol File:

// Contains sources, schemas, strings, and symbols

// -------------
// Header
// Source Table
// Format Blob
// Schema Blob 
// File Blob
// -------------

// Header's magic number is "BSL SYM" 

// Format strings, schemas, and file strings don't strictly have to be separate; they can be interleaved
// Interleaving could improve locality

struct Source {
	u64 offset_format; // Offset of format string relative to start of file
	u64 offset_schema; // Offset of schema relative to start of file
	u64 offset_file;   // Offset of file string relative to start of file
	u64 line;
	u8 level;
	u8 _pad;
};

struct SourceTable {
	u32 count; // Maximum of U32_MAX sources are supported
	Source sources[];
};

struct SchemaI {
	u8 count; // Maximum of 255 arguments are supported for a single call site
	Type types[];
};

// Format strings are NUL-terminated in blob storage
// Schemas are length-prefixed in blob storage
// File strings are NUL-terminated in blob storage

// Log File:

// Contains events and arguments for a single thread

// -------------
// Header
// Event List
// -------------

// Header's magic number is "BSL LOG" 

struct Event {
	u32 size; // Total event size, maximum U32_MAX bytes
	u32 source; // Reference to event source (contains format string and additional information)
	u8 arguments[]; // Argument instances
};

// An event is valid only if its complete size bytes are available within the file.

class Symbol {
	Symbol(const Symbol &) = delete;
	Symbol &operator =(const Symbol &) = delete;

private:
	int _fd = -1;

public:
	Symbol() {}

	bool create(const char *path) {
		if (!path)
			return false;
	}
};

class Logger {
	Logger(const Logger &) = delete;
	Logger &operator =(const Logger &) = delete;

private:
	int _fd = -1;
	Level _enabled = Level::DEFAULT;
	
public:
	Logger() = default;

	void enable(Level level) {
		_enabled = Level(u8(_enabled) | u8(level));
	}

	void disable(Level level) {
		_enabled = Level(u8(_enabled) & ~u8(level));
	}

	bool enabled(Level level) {
		return u8(_enabled) & u8(level);
	}


	bool write(Level level, ) {}
};

static Symbol symbol;
static thread_local Logger logger;

// Logger files can be initialized by successful symbol file initialization

// Public API:

}