#ifndef DESCENT_TYPE_UUID_H
#define DESCENT_TYPE_UUID_H

#include <descent/type/core.h>

typedef union {
	u8 data[16];
	u128 value;
} uuid;

#endif