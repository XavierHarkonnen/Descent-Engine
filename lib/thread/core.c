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

#define _GNU_SOURCE

#include <descent/thread/core.h>

#include <asm/unistd.h>

#include <descent/memory.h>
#include <descent/sys.h>
#include <descent/thread/core.h>
#include <descent/thread/mutex.h>
#include <descent/type/bits.h>
#include <descent/type/core.h>

#define THREAD_POOL_CAPACITY 256

struct thread_data {
	void *stack; 
	pid_t pid;
};

struct thread_pool {
	u8 count;
	struct thread_data data[THREAD_POOL_CAPACITY];
};

static _Thread_local struct thread_data *self;

static _Thread_local bool main_thread = false;

static ;

__attribute__((constructor))
static void thread_init(void) {
	main_thread = true;
}

bool thread_is_main(void) {
	return main_thread;
}


#include <linux/sched.h>
#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

// A thread ID is stale if its generation does not match the current generation of its index.
// Core pinning
// Core enumeration
// By default, clone does not set up TLS

/*
| Local | TLS and metadata
|-------|
| Stack | Stack memory
|-------|
| Guard | Guard page
*/

#define THREAD_GENERATION_INVALID 0
#define THREAD_GENERATION_START 1

#define THREAD_ID(index, generation) (((u32) (generation) & 0xFFFFFF) | ((u32) (index) << 24))
#define THREAD_ID_GENERATION(id) ((u32) (id) & 0xFFFFFF)
#define THREAD_ID_INDEX(id) ((u32) (id)  >> 24)

_Static_assert(THREAD_ID_INVALID == THREAD_ID(THREAD_INDEX_MAIN, THREAD_GENERATION_INVALID), "THREAD_ID_INVALID does not match expected layout");
_Static_assert(THREAD_ID_MAIN == THREAD_ID(THREAD_INDEX_MAIN, THREAD_GENERATION_START), "THREAD_ID_MAIN does not match expected layout");

struct thread_data {
	void              *local_base;
	void              *stack_base;
	void              *guard_base;
	u64                local_size;
	u64                stack_size;
	u64                guard_size;
};

struct thread_meta {
	u32 generation;
	pid_t tid;
	struct thread_data *data;
};

struct thread_registry {
	struct thread_meta meta[MAX_THREADS];
	struct mutex lock;
	u64 free_head;
	u8 free_stack[MAX_THREADS];
};

static struct thread_registry thread_registry = {0};

static inline bool thread_id_is_invalid(u32 id) {
	if (THREAD_ID_INDEX(id) == THREAD_INDEX_MAIN)
		return THREAD_ID_GENERATION(id) != THREAD_GENERATION_START;

	return THREAD_ID_GENERATION(id) == THREAD_GENERATION_INVALID;
}

__attribute__((constructor))
static void thread_registry_init(void) {
	for (u64 i = 0; i < MAX_THREADS; ++i) {
		thread_registry.free_stack[i] = (u8) i;
	}

	thread_registry.meta[THREAD_INDEX_MAIN].generation = THREAD_GENERATION_START;
	thread_registry.meta[THREAD_INDEX_MAIN].tid = gettid();
	thread_registry.free_head = 1;
}

u32 thread_registry_reserve(struct thread_data *data) {
	if (!data)
		return THREAD_ID_INVALID;

	u32 result = THREAD_ID_INVALID;

	mutex_lock(&thread_registry.lock);

	if (thread_registry.free_head < MAX_THREADS) {
		u8 index = thread_registry.free_stack[thread_registry.free_head++];

		sys_assert(index != THREAD_INDEX_MAIN, "Thread thread_registry must never return the main index slot");

		u32 generation = thread_registry.meta[index].generation + 1;
		u32 id = THREAD_ID(index, generation);

		if (thread_id_is_invalid(id))
			sys_fatal("Thread ID generation counter exhausted");

		thread_registry.meta[index].generation = generation;
		thread_registry.meta[index].tid = 0;
		thread_registry.meta[index].data = data;
		result = id;
	}

	mutex_unlock(&thread_registry.lock);

	return result;
}

void thread_registry_release(u32 id) {

}

u64 thread_id(void) {
	return thread_self.id;
}

u64 thread_create(i32 (*function)(void *), void *arguments, u64 stack_size) {
	if (!function)
		return THREAD_ID_INVALID;

	u64 granularity = mem_alloc_granularity();
	u64 full_size = stack_size + granularity * 2;

	void *stack = mem_alloc(&full_size, MEM_EDIT);
	if (!stack)
		return THREAD_ID_INVALID;

	if (!mem_protect(stack + full_size - granularity, granularity, MEM_NONE)) {
		mem_free(stack, full_size);
		return THREAD_ID_INVALID;
	}

	// CLONE_CHILD_CLEARTID
	// CLONE_CHILD_SETTID

	// Temporary
	struct thread_meta meta;

	struct clone_args clone_args = {0};
	clone_args.flags = CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID | CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SETTLS;
	clone_args.pidfd;
	clone_args.child_tid = (uintptr_t) &meta.pid;
	clone_args.parent_tid;
	clone_args.exit_signal;
	clone_args.stack = (uintptr_t) stack;
	clone_args.stack_size = stack_size;
	clone_args.tls;
	clone_args.set_tid;
	clone_args.set_tid_size;
	clone_args.cgroup;

	syscall(SYS_clone3, &clone_args, sizeof(clone_args));
}