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

#ifndef DESCENT_THREAD_CORE_H
#define DESCENT_THREAD_CORE_H

#include <descent/type/core.h>

static inline void spin_pause(void) {
	__asm__ __volatile__( "pause" : : : "memory" );
}

bool thread_is_main(void);

// API for creating arbitrary pools of threads that each run the same
// task in parallel, with the same rough lifespan. Each pool can be
// dynamically resized. Single-element pools are basically just threads.
// At most 256 thread pools can be created. Thread pools can contain at most 255 threads
// A thread function must return true if it wants to exit, or false if it wants to repeat.

// 8 bits index, 24 bits of generations starting at 1 (0 is invalid) 

#define THREAD_POOL_MAXIMUM 256
#define THREAD_POOL_CAPACITY 256
#define THREAD_POOL_INVALID 0

u32 thread_pool_create(u8 size, bool (*function)(void *), void *arguments);

u8 thread_pool_size(u32 pool);

bool thread_pool_resize(u32 pool, u8 size);

bool thread_pool_stop(u32 pool);

#endif