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

#ifndef DESCENT_LIB_THREAD_TASK_WORKER_HPP
#define DESCENT_LIB_THREAD_TASK_WORKER_HPP

extern "C" {
#include <pthread.h>
}

#include <descent/random.hpp>
#include <descent/thread/futex.hpp>
#include <descent/type/core.hpp>

#include "allocator.hpp"
#include "deque.hpp"
#include "injector.hpp"
#include "state.hpp"

namespace descent::thread::task {

class Worker {
public:

	enum : u32 {
		STATE_WAIT_POOL,
		STATE_WAIT_FRAME,
		STATE_CONTINUE,
		STATE_TERMINATE,
	};

private:
	// The state word holds the frame number above the state. start() advances
	// the frame, so a worker waiting for a new frame can tell that one began
	// even if it was stopped again before the worker ran.
	static constexpr u32 STATE_BITS = 2;
	static constexpr u32 STATE_MASK = (u32(1) << STATE_BITS) - 1;

	Allocator _allocator;
	Injector _injector;
	Deque _deque;
	pthread_t _pthread;
	Futex _state;

	// The frame current when the worker was configured. Read by the worker
	// thread at startup, so a frame started before the thread first runs is
	// still seen as new.
	Atomic<u32> _origin;

	random::WyRand rng;

	Futex *_finished;
	Worker *_workers;
	u64 _count;
	u64 _index;

	Worker &victim() {
		sys_assert(_workers, "Called victim before initializing worker threads");

		if (_count <= 1)
			return *this;

		const u64 offset = rng.get<u64>(1, _count);
		return _workers[(_index + offset) % _count];
	}

	static u32 state_of(u32 word) {
		return word & STATE_MASK;
	}

	static u32 frame_of(u32 word) {
		return word >> STATE_BITS;
	}

	u32 state() {
		return state_of(_state.word.load(atomic::Order::ACQUIRE));
	}

	u32 frame() {
		return frame_of(_state.word.load(atomic::Order::ACQUIRE));
	}

	void wait(u32 state) {
		for (;;) {
			const u32 word = _state.word.load(atomic::Order::ACQUIRE);
			if (state_of(word) != state)
				return;

			_state.wait(word);
		}
	}

	// Waits until a frame after `frame` has started or the worker is told to
	// terminate. Returns the current frame.
	u32 wait_frame(u32 frame) {
		for (;;) {
			const u32 word = _state.word.load(atomic::Order::ACQUIRE);
			if (frame_of(word) != frame || state_of(word) == STATE_TERMINATE)
				return frame_of(word);

			_state.wait(word);
		}
	}

	// Only the pool's controlling thread changes the state word.
	void transition(u32 state, bool next_frame) {
		const u32 next = frame() + (next_frame ? 1 : 0);
		_state.word.store((next << STATE_BITS) | state, atomic::Order::RELEASE);
		_state.wake_one();
	}

	static void *routine(void *argument) {
		Worker *worker = static_cast<Worker *>(argument);
		sys_assert(worker, "Routine called with null worker");

		self(worker);
		Worker &self = *worker;

		self.wait(STATE_WAIT_POOL);

		u32 seen = self._origin.load(atomic::Order::RELAXED);

	new_frame:

		seen = self.wait_frame(seen);

		self._injector.drain();

	executing:

		for (;;) {
			Task *task = self._deque.pop();
			if (!task)
				break;

			(*task)();
		}

		do {
			Worker &victim = self.victim();
			Task *task     = victim._deque.steal();
			if (task) {
				(*task)();
				goto executing;
			}
		} while (poll() != 0);

		// All work is done, none can be added until a new frame is started.

		for (;;) {
			switch (self.state()) {
				case STATE_WAIT_FRAME:
					self._finished->word.fetch_add(1, atomic::Order::RELEASE);
					self._finished->wake_one();
					goto new_frame;
				case STATE_TERMINATE:
					self._finished->word.fetch_add(1, atomic::Order::RELEASE);
					self._finished->wake_one();
					return nullptr;
				default: self.wait(STATE_CONTINUE); break;
			}
		}
	}

public:
	Worker() : _state(STATE_WAIT_POOL), _workers(nullptr), _count(0), _index(0) {}

	bool init() {
		// A worker restarted after set_down() still holds STATE_TERMINATE, which
		// would let its new thread exit before configure() runs.
		_state.word.store((frame() << STATE_BITS) | STATE_WAIT_POOL, atomic::Order::RELAXED);

		return pthread_create(&_pthread, nullptr, routine, this) == 0;
	}

	void configure(Futex *finished, Worker *workers, u64 count, u64 index) {
		rng.seed(index);

		_finished = finished;
		_workers  = workers;
		_count    = count;
		_index    = index;
		// Published to the worker by the release store in transition().
		_origin.store(frame(), atomic::Order::RELAXED);
		transition(STATE_WAIT_FRAME, false);
	}

	void terminate() {
		transition(STATE_TERMINATE, false);
	}

	void join() {
		pthread_join(_pthread, nullptr);
	}

	void start() {
		transition(STATE_CONTINUE, true);
	}

	void inject(const Job &job, Root &root) {
		_injector.push(job, root);
	}

	void stop() {
		transition(STATE_WAIT_FRAME, false);
	}

	void clear() {
		_allocator.clear();
	}

	Task *allocate() {
		return _allocator.allocate();
	}

	void submit(Task *task) {
		_deque.push(task);
	}
};

}

#endif
