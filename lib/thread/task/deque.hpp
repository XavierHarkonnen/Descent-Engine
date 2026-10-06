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

#ifndef DESCENT_LIB_THREAD_TASK_DEQUE_HPP
#define DESCENT_LIB_THREAD_TASK_DEQUE_HPP

#include <descent/thread/atomic.hpp>
#include <descent/type/core.hpp>

#include "node.hpp"

namespace descent::thread::task {

class Deque {
private:
	static constexpr u64 CAPACITY = 1024;
	static_assert((CAPACITY & (CAPACITY - 1)) == 0, "CAPACITY must be a power of two");

	Atomic<Task *>tasks[CAPACITY];
	
	Atomic<u64> top;
	Atomic<u64> bottom;

public:

	Deque() : top(0), bottom(0) {}

	void push(Task *task) {
		u64 b = bottom.load(atomic::Order::RELAXED);
		u64 t = top.load(atomic::Order::ACQUIRE);

		if (b - t >= CAPACITY) {
			// If there's no capacity, execute the task inline
			(*task)();
			return;
		}

		tasks[b & (CAPACITY - 1)].store(task, atomic::Order::RELAXED);

		atomic::memory_barrier(atomic::Order::RELEASE);

		bottom.store(b + 1, atomic::Order::RELAXED);
	}

	Task *pop() {
		u64 b = bottom.load(atomic::Order::RELAXED) - 1;
		bottom.store(b, atomic::Order::RELAXED);

		atomic::memory_barrier(atomic::Order::SEQ_CST);

		u64 t = top.load(atomic::Order::RELAXED);

		i64 count = static_cast<i64>(b - t);
		if (count < 0) {
			// Deque was already empty, update the bottom to canonical empty state
			bottom.store(b + 1, atomic::Order::RELEASE);
			return nullptr;
		}

		Task *task = tasks[b & (CAPACITY - 1)].load(atomic::Order::RELAXED);

		if (count > 0) {
			// More than one task was left in deque at acquire, so thieves
			// cannot and could not interfere with this task as they all
			// compete with each another for the one task at the top.
			return task;
		}

		// Compete with thieves for the last task in deque
		if (!top.compare_exchange(t, t + 1, atomic::Order::SEQ_CST, atomic::Order::RELAXED)) {
			// Lost the race with a thief, task is invalid
			task = nullptr;
		}

		// Win or lose, update the bottom to canonical empty state
		bottom.store(b + 1, atomic::Order::RELAXED);
		
		return task;
	}

	Task *steal() {
		u64 t = top.load(atomic::Order::ACQUIRE);

		atomic::memory_barrier(atomic::Order::SEQ_CST);

		u64 b = bottom.load(atomic::Order::ACQUIRE);

		i64 count = static_cast<i64>(b - t);
		if (count <= 0) {
			return nullptr;
		}

		Task *task = tasks[t & (CAPACITY - 1)].load(atomic::Order::RELAXED);
		
		// Verify that another thread has not taken the work
		if (!top.compare_exchange(t, t + 1, atomic::Order::SEQ_CST, atomic::Order::RELAXED)) {
			return nullptr;
		}
		
		// Task is now known to be valid and can be returned
		return task;
	}
};

}

#endif