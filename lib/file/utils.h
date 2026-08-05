#ifndef DESCENT_SOURCE_FILE_UTILS_H
#define DESCENT_SOURCE_FILE_UTILS_H

#include <descent/type/core.h>

bool make_raw_dir(const char *path);
bool make_raw_dirs(const char *path);
int open_raw_dir(const char *path);
bool make_dir(int root, const char *path);
bool make_dirs(int root, const char *path);
int open_dir(int root, const char *path);

#endif
