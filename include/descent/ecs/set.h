#ifndef DESCENT_ECS_SET_H
#define DESCENT_ECS_SET_H

#include <descent/type/core.h>
#include <descent/type/bits.h>
#include <descent/utils.h>

#define ECS_SET_WORDS 4

struct ecs_set {
	_Alignas(sizeof(u64) * ECS_SET_WORDS)
	u64 _[ECS_SET_WORDS];
};

// Returns a set containing only the element e
static inline struct ecs_set ecs_set_bit(u8 e) {
	struct ecs_set s = {0};
	s._[e / 64] = (1ull << ((e) % 64));
	return s;
}

// Internal function
static inline struct ecs_set ECS_SET_BITS_INTERNAL(u64 count, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, count);

	struct ecs_set s = {0};

	for (u64 i = 0; i < count; ++i) {
		u8 element = (u8) __builtin_va_arg(args, unsigned);
		s._[(element) / 64] |= (1ull << ((element) % 64));
	}

	__builtin_va_end(args);

	return s;
}

// Returns a set containing all the provided elements
#define ecs_set_bits(...) ECS_SET_BITS_INTERNAL(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

// Returns true if the set s is empty, or false otherwise
static inline bool ecs_set_empty(struct ecs_set s) {
	u64 bits = 0;

	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		bits |= s._[i];

	return bits == 0;
}

// Returns the number of elements in the set s
static inline u64 ecs_set_size(struct ecs_set s) {
	u64 result = 0;

	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		result += popcnt_64(s._[i]);

	return result;
}

// Returns the set s with the element e added
static inline struct ecs_set ecs_set_add(struct ecs_set s, u8 e) {
	s._[(e) / 64] |= 1ull << ((e) % 64);
	return s;
}

// Returns the set s with the element e removed
static inline struct ecs_set ecs_set_remove(struct ecs_set s, u8 e) {
	s._[(e) / 64] &= ~(1ull << ((e) % 64));
	return s;
}

// Returns true if set s contains the element e, or false otherwise
static inline bool ecs_set_contains(struct ecs_set s, u8 e) {
	return s._[(e) / 64] & (1ull << ((e) % 64));
}

// Returns the complement of the set s
static inline struct ecs_set ecs_set_complement(struct ecs_set s) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		s._[i] = ~s._[i];

	return s;
}

// Returns the union of sets a and b
static inline struct ecs_set ecs_set_union(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		a._[i] |= b._[i];

	return a;
}

// Returns the intersection between sets a and b
static inline struct ecs_set ecs_set_intersection(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		a._[i] &= b._[i];

	return a;
}

// Returns the difference between set a and set b
static inline struct ecs_set ecs_set_difference(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		a._[i] &= ~b._[i];

	return a;
}

// Returns the symmetric difference between sets a and b
static inline struct ecs_set ecs_set_symmetric_difference(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		a._[i] ^= b._[i];

	return a;
}

// Returns true if set a equals set b, or false otherwise
static inline bool ecs_set_equals(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		if (a._[i] != b._[i])
			return false;

	return true;
}

// Returns true if set a is a subset of set b, or false otherwise
static inline bool ecs_set_subset(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		if (a._[i] & ~b._[i])
			return false;

	return true;
}

// Returns true if set a intersects set b, or false otherwise
static inline bool ecs_set_intersects(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		if (a._[i] & b._[i])
			return true;

	return false;
}

// Returns true if set a is disjoint with set b, or false otherwise
static inline bool ecs_set_disjoint(struct ecs_set a, struct ecs_set b) {
	for (u64 i = 0; i < ECS_SET_WORDS; ++i)
		if (a._[i] & b._[i])
			return false;

	return true;
}

#undef ECS_SET_WORDS

#endif