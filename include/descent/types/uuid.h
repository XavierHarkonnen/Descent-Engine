#ifndef DESCENT_UUID_H
#define DESCENT_UUID_H

#include <descent/types/core.h>

typedef union {
	u8 data[16];
	u128 value;
} uuid;

#endif