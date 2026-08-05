#include "bsl.h"

#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <descent/type/core.h>
#include <descent/utils.h>

#define BSL_SIGNATURE  0x454C494620534C42ull

struct bsl_site_data {
	const char *_file;
	const char *_message;
	u32         _line;
	u8          _module;
	u8          _level;
	u16         _pad0;
	u64         _pad1;
};

static const struct bsl_site_data *bsl_sites;
static u32 bsl_site_count;

__attribute__((constructor))
static void bsl_init(void) {
	extern const struct bsl_site_data __start_bsl[];
	extern const struct bsl_site_data __stop_bsl[];

	bsl_sites = __start_bsl;

	ptrdiff_t bsl_diff = __stop_bsl - __start_bsl;

	if (bsl_diff >= U32_MAX) {
		const char *message = "FATAL ERROR: Too many BSL sites (matched or exceeded U32_MAX)";
		write(STDERR_FILENO, message, strlen(message));
		_Exit(-1);
	}

	bsl_site_count = (u32) (bsl_diff);
}

static inline u32 bsl_site_index(const struct bsl_site_data *site) {
	return (u32) (site - bsl_sites);
}

void bsl_create_directory() {
	int result = mkdir("~/.local/state/descent/", 0644);
	/*
	openat
	fstatat
	mkdirat
	unlinkat
	renameat
	linkat
	symlinkat
	faccessat
	*/
}

void bsl_create_metadata();

void bsl_create_segment();

void bsl_create_session(void) {
	// Create the session directory
	// Create the metadata file
}

// Standard formatting variant of BSL_LOG
#define BSL_LOG_STDFMT(level, module, message, ...) do {                                     \
	static const struct bsl_site_data __attribute__((section("bsl"), used)) bsl_site_data = { \
		._file    = __FILE__,                                                                    \
		._message = message,                                                                     \
		._line    = __LINE__,                                                                    \
		._module  = module,                                                                      \
		._level   = level                                                                        \
	};                                                                                         \
	/* bsl_emit(&bsl_site_data, COUNT_ARGS(__VA_ARGS__), __VA_OPT__(,) __VA_ARGS__); */        \
} while (0)

int main() {
	BSL_LOG_STDFMT(0, 0, "m1");
	BSL_LOG_STDFMT(0, 0, "m2");
	BSL_LOG_STDFMT(0, 0, "m3");

	printf("Site Count: %u\n", bsl_site_count);
	for (u32 i = 0; i < bsl_site_count; ++i) {
		printf("Site %u\n", i);
		printf("  file: %s\n", bsl_sites[i]._file);
		printf("  message: %s\n", bsl_sites[i]._message);
		printf("  line: %u\n", bsl_sites[i]._line);
		printf("  module: %u\n", bsl_sites[i]._module);
		printf("  level: %u\n", bsl_sites[i]._level);
	}

	return 0;
}

void help(void) {
	BSL_LOG_STDFMT(0, 0, "m0");
}
