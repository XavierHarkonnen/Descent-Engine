#ifndef DESCENT_ECS_H
#define DESCENT_ECS_H

#include <descent/rcode.h>
#include <descent/type/core.h>
#include <descent/utils.h>
#include <descent/type/bits.h>
#include <descent/type/core.h>
#include <descent/ecs/component.h>

// TODO: move
struct vec3 {
	u32 x;
	u32 y;
	u32 z;
};

// Constants

#define ECS_MAX_COMPONENTS    256
#define ECS_MAX_ARCHETYPES    256
#define ECS_MAX_SEGMENTS      32768
#define ECS_SEGMENT_BYTES    (32 * 1024)
#define ECS_STORAGE_BYTES    (ECS_MAX_SEGMENTS * ECS_SEGMENT_BYTES)
#define ECS_INVALID_SEGMENT  -1

#define ECS_COMPONENT_X_LIST \
	X(position,  ECS_COMPONENT_POSITION,  struct vec3) \
	X(rotation,  ECS_COMPONENT_ROTATION,  struct vec3) \
	X(velocity,  ECS_COMPONENT_VELOCITY,  struct vec3) \
	X(transform, ECS_COMPONENT_TRANSFORM, struct vec3) \
	X(mesh,      ECS_COMPONENT_MESH,      void *) \
	X(health,    ECS_COMPONENT_HEALTH,    u16) \

enum {
#define X(name, id, type) id,
	ECS_COMPONENT_X_LIST
#undef X
	ECS_COMPONENT_COUNT
};

enum {
#define X(name, id, type) CONCAT(id, _SIZE) = sizeof(type),
	ECS_COMPONENT_X_LIST
#undef X
};

enum {
#define X(name, id, type) CONCAT(id, _ALIGN) = _Alignof(type),
	ECS_COMPONENT_X_LIST
#undef X
};

#define ECS_SIZEOF(component) CONCAT(component, _SIZE)
#define ECS_ALIGNOF(component) CONCAT(component, _ALIGN)

_Static_assert(ECS_COMPONENT_COUNT <= 256, "At most 256 ECS components are supported");

// Types

typedef u64 ecs_entity;
typedef u8  ecs_phase;

typedef struct ecs_system_data ecs_system_data;
typedef struct ecs_segment     ecs_segment;
typedef struct ecs_system      ecs_system;

typedef void (*ecs_system_kernel)(struct ecs_system_data *system_data, void *user_data);

// Command API

/*
These commands can only be called during the command phase.

Resource exhaustion is transactional. If the operation returns fewer than the
number of valid entities because of resource exhaustion, no entities were
modified.

Invalid IDs are ignored.

Restrict commands to only run on a specialized ECS thread; this means commands cannot be run during the
systems run because the scheduler takes over the thread. This vastly simplifies command control, and
broadly parallel operations *should* be set up through the systems interface instead of the command
interface.

Can have a command queue which the main thread drains asynchronously before each systems run.
*/

// Destroys all entities, resets state to empty/default
void ecs_clear(void);

// Creates entities with a set of components.
bool ecs_create(ecs_components components, u32 count, ecs_entity *entities);

// Destroys entities.
bool ecs_destroy(const ecs_entity *entities, u32 count);

// Adds a set of components to entities.
bool ecs_add_components(const ecs_entity *entities, u32 count, ecs_components components);

// Removes a set of components from entities
bool ecs_remove_components(const ecs_entity *entities, u32 count, ecs_components components);

// Alters (adds and removes) a set of components to/from entities
bool ecs_alter_components(const ecs_entity *entities, u32 count, ecs_components add, ecs_components remove);

// Defines the set of components for entities
bool ecs_define_components(const ecs_entity *entities, u32 count, ecs_components components);

// Checks if an entity exists
bool ecs_exists(ecs_entity entity);

// Checks if an entity has a set of components
bool ecs_has(ecs_entity entity, ecs_components components);

// Gets a single component from a single entity
void *ecs_get(ecs_entity entity, ecs_component component);

#define X(name, id, type) static inline type *CONCAT(ecs_get_, name)(ecs_entity entity) { return ecs_get(entity, id); }
ECS_COMPONENT_X_LIST
#undef X

// Systems Control API

// A system should look like a kernel
// Attempting to call a command from a systems kernel fails (returns false or terminates)
// Operations in a given phase run in an undefined order, and should not depend on the output of other systems in that phase

ecs_system *ecs_register_system(ecs_system_kernel kernel, ecs_phase phase, ecs_components require, ecs_components exclude, void *user_data);

void ecs_destroy_system(ecs_system *system);

// Triggers a systems run, blocks until complete.
// Commands cannot be executed until the systems run is complete.
void ecs_run(void);

// Systems Interface API

// Tools for writing systems
// Requesting an invalid component terminates

// Returns NULL when done
ecs_segment *ecs_next_segment(ecs_system_data *system_data);

u16 ecs_component_count(ecs_segment *segment);

void *ecs_span(ecs_segment *segment, ecs_component component);

#define X(name, id, type) \
	static inline type *CONCAT(ecs_span_, name)(ecs_segment *segment) { return ecs_span(segment, id); }
ECS_COMPONENT_X_LIST
#undef X

#undef ECS_COMPONENT_X_LIST

#endif