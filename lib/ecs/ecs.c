#include <descent/ecs.h>

#include <sys/mman.h>
#include <unistd.h>

#include <descent/sys.h>
#include <descent/thread/atomic.h>
#include <descent/thread/mutex.h>
#include <descent/type/core.h>
#include <descent/ecs/set.h>

#define ECS_PAGE_ALIGNMENT 4096

// Entity IDs: 16 bits of index, 32 bits of generation, local vs remote
// #define ENTITY_ID(index, generation) (((u64) index << 32) | ((u64) generation & 0xFFFFFFFFu))
// #define ENTITY_ID_INDEX(id) ((u32) (id >> 32))
// #define ENTITY_ID_GENERATION(id) ((u32) (id & 0xFFFFFFFFu))

// TODO: Move
static inline void *sysalloc(u64 size) {
	void *alloc = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return (alloc == MAP_FAILED) ? NULL : alloc;
}

// TODO: Move
static inline void sysfree(void *alloc, u64 size) {
	munmap(alloc, size);
}

enum {
	ECS_MODE_COMMAND,
	ECS_MODE_SYSTEMS
};

struct ecs_segment {
	u8 data[ECS_SEGMENT_BYTES];
};

struct ecs_archetype {
	// Component Data
	struct ecs_set components;

	// Command-Mode Lock
	struct mutex mutex;

	// Segment Storage
	u16 active_segment_index;
	u16 active_segment_entity_count;
	u16 owned_segment_count;

	// Segment Layout Data
	u16 segment_offsets[ECS_MAX_COMPONENTS];
	u16 segment_bytes_per_entity;
	u8 segment_entity_capacity;

	// Cached metadata
	u8 component_count;

	// Padding
	u8 _pad[50];
};

struct ecs_scheduler {
	u32 mode;
};

// Stack for allocating free segments
struct ecs_segment_stack {
	u64 head;
	struct mutex mutex;
	i16 stack[ECS_MAX_SEGMENTS];
};

// ECS world
struct ecs {
	// Backing storage for segments
	_Alignas(ECS_PAGE_ALIGNMENT) // 1 GiB, 32768 x 32 kiB
	struct ecs_segment segments[ECS_MAX_SEGMENTS];

	// Table of segments owned by each archetype
	_Alignas(ECS_PAGE_ALIGNMENT) // 16 MiB, 256 x 32768 x 2 B
	i16 archetype_segment_indices[ECS_MAX_ARCHETYPES][ECS_MAX_SEGMENTS];

	// Table for looking up a set of archetypes possessing a set of components
	_Alignas(ECS_PAGE_ALIGNMENT) // 16 kiB, 256 x 64 B
	struct ecs_set archetype_lookup[ECS_MAX_COMPONENTS];

	// 160 kiB, 256 x 640 B
	struct ecs_archetype archetypes[ECS_MAX_ARCHETYPES];

	// 65552 B
	struct ecs_segment_stack segment_stack;

	// 4 B
	struct ecs_scheduler control;
};

static struct ecs *ecs_create(void) {
	struct ecs *ecs = sysalloc(sizeof(struct ecs));
	if (!ecs)
		return NULL;

	for (u64 i = 0; i < ECS_MAX_SEGMENTS; ++i)
		ecs->segment_stack.stack.segments[i] = (i16) i;

	return ecs;
}

static void ecs_destroy(struct ecs *ecs) {
	if (!ecs)
		return;

	sysfree(ecs, sizeof(struct ecs));
}


static i16 ecs_segment_stack_pop(void) {
	i16 index = ECS_INVALID_SEGMENT;

	mutex_lock(&ecs_segment_stack_mutex);

	if (ecs_segment_stack_head < ECS_MAX_SEGMENTS)
		index = ecs_segment_stack.segments[ecs_segment_stack_head++];

	mutex_unlock(&ecs_segment_stack_mutex);
	
	return index;
}

static void ecs_segment_stack_push(i16 index) {
	mutex_lock(&ecs_segment_stack_mutex);

	sys_assert(ecs_segment_stack_head > 0, "All ECS segment stack push operations must match a preceding valid pop operation");
	ecs_segment_stack.segments[--ecs_segment_stack_head] = index;

	mutex_unlock(&ecs_segment_stack_mutex);
}


/*
// Entity ID API
// For specific entities

// Archetype API
// For iterating over objects with properties

// World API

// System scheduler handles multithreading, not the systems themselves

// Separate into system phase and command phase
// Structural and individual changes are commands

// Command API

*/
