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

#ifndef DESCENT_THREAD_TASK_HPP
#define DESCENT_THREAD_TASK_HPP

#include <descent/type/core.hpp>

namespace descent::thread::task {

static inline constexpr u64 INLINE_DATA_SIZE = 36;
static inline constexpr u64 MAX_START_JOBS = 64;
static inline constexpr u64 MAX_WORKERS = 16;

class Handle;
class Task;

using Routine = void (*)(Handle self, void *);

struct Job {
	Routine routine;
	void *context;
};

// A handle is valid only during execution of the task to which it was passed.
// Using a handle after the frame in which it was passed is undefined behavior.
class Handle {
	friend class Task;

	Handle(const Handle &other) = delete;
	Handle &operator=(const Handle &other) = delete;

private:
	Task *_;

	Handle(Task *task) : _(task) {}

	Handle(Handle &&other) : _(other._) {
		other._ = nullptr;
	}

	Handle &operator=(Handle &&other) {
		if (this != &other) {
			_ = other._;
			other._ = nullptr;
		}

		return *this;
	}

public:
	Handle() : _(nullptr) {}

	explicit operator bool() const {
		return _ != nullptr;
	}

	// Spawns job as a child of the current task. Returns false if the job could
	// not be spawned. A failed spawn does not create a child task.
	bool spawn(Job job);
};

// If the task frame is inactive, sets the number of task worker threads.
// Returns the current total number of worker threads.
u64 set(u64 count);

// Returns the total number of task worker threads.
u64 get();

bool begin(const Job *jobs, u64 count);

// If the task frame is inactive, activates it and returns true. Otherwise
// returns false.
template <u64 N>
static bool begin(const Job (&jobs)[N]) {
	static_assert(N > 0 && N <= MAX_START_JOBS, "Illegal starting job count");
	return begin(jobs, N);
}

// Return the number of unfinished tasks. Always returns zero if the task
// frame is inactive.
u32 poll();

// Waits until the number of unfinished tasks is zero.
void wait();

// If the task frame is active and contains no active tasks, deactivates it
// and returns true. Otherwise returns false.
bool end();
}

#endif