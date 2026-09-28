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

#include <descent/type.hpp>

namespace descent::thread::task {

static constexpr u64 INLINE_DATA_SIZE = 36;

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

	// Spawns job as a child of the current task.
	// Returns false if the job could not be spawned.
	// A failed spawn does not create a child task.
	bool spawn(Job job);
};

struct frame {
private:
	static bool begin(const Job *jobs, u64 count);

public:
	static constexpr u64 MAX_START_JOBS = 64;

	// If the frame is inactive, activates it and returns true.
	// Otherwise returns false.
	template <u64 N>
	static bool begin(const Job (&jobs)[N]) {
		static_assert(N > 0 && N <= MAX_START_JOBS, "Illegal starting job count");

		for (const Job &job : jobs)
			if (!job.routine)
				return false;

		return begin(jobs, N);
	}

	// Return the number of unfinished tasks.
	// Always returns zero if the frame is inactive.
	static u32 poll();

	// Waits until the number of unfinished tasks is zero.
	static void wait();

	// Check completion and, while incomplete, execute available work on the calling thread
	//static u32 help();

	// If the frame is active and contains no active tasks, deactivates it and returns true.
	// Otherwise returns false.
	static bool end();
};

namespace diagnostic {
	// Returns the total number of task worker threads
	u64 workers();

	//u64 active_workers();
	//u64 sleeping_workers();
	// Per-thread:
	//tasks_executed
	//tasks_stolen
	//steal_attempts
	//failed_steals
}

}

#endif