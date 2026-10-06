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
	Allocator _allocator;
	Injector _injector;
	Deque _deque;
	pthread_t _pthread;
	Futex _state;

	random::WyRand rng;

	Futex *_finished;
	Worker *_workers;
	u64 _count;
	u64 _index;

	Worker &victim() {
		sys_assert(_workers, "Called victim before initializing worker threads");

		if (_count <= 1)
			return *this;

		const u64 offset = rng.get<u64>(1, _count - 1);
		return _workers[(_index + offset) % _count];
	}

	u32 state() {
		return _state.word.load(atomic::Order::ACQUIRE);
	}

	void wait(u32 state) {
		while (this->state() == state)
			_state.wait(state);
	}

	static void *routine(void *argument) {
		Worker *worker = static_cast<Worker *>(argument);
		sys_assert(worker, "Routine called with null worker");

		self(worker);
		Worker &self = *worker;

		self.wait(STATE_WAIT_POOL);
		
	new_frame:

		self.wait(STATE_WAIT_FRAME);

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
			Task *task = victim._deque.steal();
			if (task) {
				(*task)();
				goto executing;
			}
		} while (poll() != 0);

		// All work is done, none can be added until a new frame is started.

		for (;;) {
			switch(self.state()) {
				case STATE_WAIT_FRAME:
					self._finished->word.fetch_add(1, atomic::Order::RELEASE);
					self._finished->wake_one();
					goto new_frame;
				case STATE_TERMINATE:
					self._finished->word.fetch_add(1, atomic::Order::RELEASE);
					self._finished->wake_one();
					return nullptr;
				default:
					self.wait(STATE_CONTINUE);
					break;
			}
		}
	}

public:
	Worker() : _state(STATE_WAIT_POOL), _workers(nullptr), _count(0), _index(0) {}

	bool init() {
		return pthread_create(&_pthread, nullptr, routine, this) == 0;
	}

	void configure(Futex *finished, Worker *workers, u64 count, u64 index) {
		rng.seed(index);

		_finished = finished;
		_workers = workers;
		_count = count;
		_index = index;
		_state.word.store(STATE_WAIT_FRAME, atomic::Order::RELEASE);
		_state.wake_one();
	}

	void terminate() {
		_state.word.store(STATE_TERMINATE, atomic::Order::RELEASE);
		_state.wake_one();
	}

	void join() {
		pthread_join(_pthread, nullptr);
	}

	void start() {
		_state.word.store(STATE_CONTINUE, atomic::Order::RELEASE);
		_state.wake_one();
	}

	void inject(const Job &job, Root &root) {
		_injector.push(job, root);
	}

	void stop() {
		_state.word.store(STATE_WAIT_FRAME, atomic::Order::RELEASE);
		_state.wake_one();
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