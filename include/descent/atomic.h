/* Copyright 2025 XavierHarkonnen9 and Enlarium
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DESCENT_ATOMIC_H
#define DESCENT_ATOMIC_H

#include <descent/build.h>
#include <descent/types.h>

enum {
	ATOMIC_RELAXED = __ATOMIC_RELAXED,
	ATOMIC_ACQUIRE = __ATOMIC_ACQUIRE,
	ATOMIC_RELEASE = __ATOMIC_RELEASE,
	ATOMIC_ACQ_REL = __ATOMIC_ACQ_REL,
	ATOMIC_SEQ_CST = __ATOMIC_SEQ_CST
};

#define atomic_load_ex(ptr, order) __atomic_load_n(ptr, order)
#define atomic_load(ptr) atomic_load_ex(ptr, ATOMIC_ACQUIRE)

#define atomic_store_ex(ptr, val, order) __atomic_store_n(ptr, val, order)
#define atomic_store(ptr, val) atomic_store_ex(ptr, val, ATOMIC_RELEASE)

#define atomic_exchange_ex(ptr, val, order) __atomic_exchange_n(ptr, val, order)
#define atomic_exchange(ptr, val) atomic_exchange_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_compare_exchange_ex(ptr, expected, desired, success_order, failure_order) __atomic_compare_exchange_n(ptr, expected, desired, false, success_order, failure_order)
#define atomic_compare_exchange(ptr, expected, desired) atomic_compare_exchange_ex(ptr, expected, desired, ATOMIC_ACQ_REL, ATOMIC_ACQUIRE)


#define atomic_add_fetch_ex(ptr, val, order) __atomic_add_fetch(ptr, val, order)
#define atomic_add_fetch(ptr, val) atomic_add_fetch_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_sub_fetch_ex(ptr, val, order) __atomic_sub_fetch(ptr, val, order)
#define atomic_sub_fetch(ptr, val) atomic_sub_fetch_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_and_fetch_ex(ptr, val, order) __atomic_and_fetch(ptr, val, order)
#define atomic_and_fetch(ptr, val) atomic_and_fetch_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_xor_fetch_ex(ptr, val, order) __atomic_xor_fetch(ptr, val, order)
#define atomic_xor_fetch(ptr, val) atomic_xor_fetch_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_or_fetch_ex(ptr, val, order) __atomic_or_fetch(ptr, val, order)
#define atomic_or_fetch(ptr, val) atomic_or_fetch_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_nand_fetch_ex(ptr, val, order) __atomic_nand_fetch(ptr, val, order)
#define atomic_nand_fetch(ptr, val) atomic_nand_fetch_ex(ptr, val, ATOMIC_ACQ_REL)


#define atomic_fetch_add_ex(ptr, val, order) __atomic_fetch_add(ptr, val, order)
#define atomic_fetch_add(ptr, val) atomic_fetch_add_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_fetch_sub_ex(ptr, val, order) __atomic_fetch_sub(ptr, val, order)
#define atomic_fetch_sub(ptr, val) atomic_fetch_sub_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_fetch_and_ex(ptr, val, order) __atomic_fetch_and(ptr, val, order)
#define atomic_fetch_and(ptr, val) atomic_fetch_and_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_fetch_xor_ex(ptr, val, order) __atomic_fetch_xor(ptr, val, order)
#define atomic_fetch_xor(ptr, val) atomic_fetch_xor_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_fetch_or_ex(ptr, val, order) __atomic_fetch_or(ptr, val, order)
#define atomic_fetch_or(ptr, val) atomic_fetch_or_ex(ptr, val, ATOMIC_ACQ_REL)

#define atomic_fetch_nand_ex(ptr, val, order) __atomic_fetch_nand(ptr, val, order)
#define atomic_fetch_nand(ptr, val) atomic_fetch_nand_ex(ptr, val, ATOMIC_ACQ_REL)


static inline bool atomic_test_and_set_ex(bool *ptr, int order) { return __atomic_test_and_set(ptr, order); }
static inline bool atomic_test_and_set(bool *ptr)               { return atomic_test_and_set_ex(ptr, ATOMIC_ACQ_REL); }

static inline void atomic_clear_ex(bool *ptr, int order) { __atomic_clear(ptr, order); }
static inline void atomic_clear(bool *ptr)               { atomic_clear_ex(ptr, ATOMIC_RELEASE); }

#endif