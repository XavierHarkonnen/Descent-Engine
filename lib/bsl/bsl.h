#ifndef DESCENT_BSL_H
#define DESCENT_BSL_H

#include <descent/type/core.h>

struct bsl_header {
	u64 signature; // always BSL_SIGNATURE
	u32 type;      // segment or metadata
	u32 version;   // specific to type
	u64 flags;     //
	u32 size;      // file size
};

struct bsl_metadata {
	struct bsl_header header;
};

#endif