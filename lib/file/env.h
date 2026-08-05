#ifndef DESCENT_SOURCE_FILE_ENV_H
#define DESCENT_SOURCE_FILE_ENV_H

#include <descent/type/core.h>

struct file_roots {
	i32 base;
	i32 config;
	i32 data;
	i32 state;
	i32 cache;
};

const struct file_roots *file_get_roots(void);

#endif