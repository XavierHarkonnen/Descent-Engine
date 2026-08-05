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

#ifndef DESCENT_THREAD_MUTEX_H
#define DESCENT_THREAD_MUTEX_H

#include <descent/thread/atomic.h>
#include <descent/thread/core.h>
#include <descent/thread/futex.h>
#include <descent/time.h>
#include <descent/type/core.h>

#define MUTEX_SPIN_COUNT 64

enum {
	MUTEX_FREE      = 0,
	MUTEX_LOCKED    = 1,
	MUTEX_CONTENDED = 2,
};

struct mutex {
	u32 _state;
};

#define MUTEX_INIT {0}

static inline bool mutex_trylock(struct mutex *m) {
	if (!m)
		return false;

	u32 expected = MUTEX_FREE;
	return atomic_compare_exchange_ex(&m->_state, &expected, MUTEX_LOCKED, ATOMIC_ACQUIRE, ATOMIC_RELAXED);
}

static inline bool mutex_timedlock(struct mutex *m, u64 duration) {
	if (!m)
		return false;

	if (mutex_trylock(m))
		return true;

	u64 start = time_now();
	u64 deadline = start + duration;

	u64 spins = MUTEX_SPIN_COUNT;
	while (spins-- && atomic_load_ex(&m->_state, ATOMIC_RELAXED) == MUTEX_LOCKED)
		spin_pause();

	for (;;) {
		u32 expected = MUTEX_FREE;
		if (atomic_compare_exchange(&m->_state, &expected, MUTEX_LOCKED))
			return true;

		if (expected == MUTEX_LOCKED)
			atomic_compare_exchange(&m->_state, &expected, MUTEX_CONTENDED);

		u64 now = time_now();
		if (duration && (now >= deadline))
			return false;

		u64 remaining = duration ? deadline - now : 0;

		expected = MUTEX_CONTENDED;
		futex_wait(&m->_state, expected, remaining);
	}
}

static inline bool mutex_lock(struct mutex *m) {
	return mutex_timedlock(m, 0);
}

static inline bool mutex_unlock(struct mutex *m) {
	if (!m)
		return false;

	u32 state = atomic_exchange_ex(&m->_state, MUTEX_FREE, ATOMIC_RELEASE);

	if (state == MUTEX_CONTENDED)
		futex_wake_single(&m->_state);

	return true;
}

#endif
