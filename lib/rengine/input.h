#ifndef DESCENT_SOURCE_RENGINE_INPUT_H
#define DESCENT_SOURCE_RENGINE_INPUT_H

#include <descent/rengine.h>


struct input_table;

struct input_tables {
	struct input_table *tables;
	struct input_table *active;
	u8 count;
};

struct rengine_context;


u64 rengine_create_input_tables(struct rengine_context *context);

void rengine_destroy_input_tables(struct rengine_context *context);

#endif