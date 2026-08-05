#include <descent/ecs/set.h>

typedef u8 ecs_component;
typedef struct ecs_set ecs_components;

// Returns a component set containing only the component c
#define ecs_component_bit(c) ecs_set_bit(c)

// Returns a component set containing all the provided components
#define ecs_component_bits(...) ecs_set_bits(__VA_ARGS__)

// Returns true if the component set c is empty, or false otherwise
static inline bool ecs_components_empty(ecs_components c) {
	return ecs_set_empty(c);
}

// Returns the number of components in the component set c
static inline u64 ecs_components_size(ecs_components c) {
	return ecs_set_size(c);
}

// Returns the component set c with the component e added
static inline ecs_components ecs_component_add(ecs_components c, ecs_component e) {
	return ecs_set_add(c, e);
}

// Returns the component set c with the component e removed
static inline ecs_components ecs_components_remove(ecs_components c, ecs_component e) {
	return ecs_set_remove(c, e);
}

// Returns true if component set c contains the component e, or false otherwise
static inline bool ecs_components_contains(ecs_components c, ecs_component e) {
	return ecs_set_contains(c, e);
}

// Returns the complement of the component set c
static inline ecs_components ecs_components_complement(ecs_components c) {
	return ecs_set_complement(c);
}

// Returns the union of component sets a and b
static inline ecs_components ecs_components_union(ecs_components a, ecs_components b) {
	return ecs_set_union(a, b);
}

// Returns the intersection between component sets a and b
static inline ecs_components ecs_components_intersection(ecs_components a, ecs_components b) {
	return ecs_set_intersection(a, b);
}

// Returns the difference between component set a and component set b
static inline ecs_components ecs_components_difference(ecs_components a, ecs_components b) {
	return ecs_set_difference(a, b);
}

// Returns the symmetric difference between component sets a and b
static inline ecs_components ecs_components_symmetric_difference(ecs_components a, ecs_components b) {
	return ecs_set_symmetric_difference(a, b);
}

// Returns true if component set a equals component set b, or false otherwise
static inline bool ecs_components_equals(ecs_components a, ecs_components b) {
	return ecs_set_equals(a, b);
}

// Returns true if set b is a subset of set a, or false otherwise
static inline bool ecs_components_subset(ecs_components a, ecs_components b) {
	return ecs_set_subset(a, b);
}

// Returns true if set a intersects set b, or false otherwise
static inline bool ecs_components_intersects(ecs_components a, ecs_components b) {
	return ecs_set_intersects(a, b);
}

// Returns true if set a is disjoint with set b, or false otherwise
static inline bool ecs_components_disjoint(ecs_components a, ecs_components b) {
	return ecs_set_disjoint(a, b);
}
