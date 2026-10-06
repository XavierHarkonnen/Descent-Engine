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

#ifndef DESCENT_LIB_THREAD_TASK_POOL_HPP
#define DESCENT_LIB_THREAD_TASK_POOL_HPP

extern "C" {
#include <sys/sysinfo.h>
}

#include <descent/thread/mutex.hpp>
#include <descent/type.hpp>

#include "worker.hpp"
#include "state.hpp"

namespace descent::thread::task {

class Pool {
private:
	static constexpr u64 MAX = 16;

	Worker _workers[MAX];
	Root _root;
	Atomic<u64> _count;
	Mutex _mutex;
	Futex _finished;
	bool _active;

	void set_up(u64 count) {
		sys_assert(count > get(), "Attempted to call set_up with desired workers less than or equal to current workers");

		u64 index;
		for (index = get(); index < count; ++index) {
			if (!_workers[index].init())
				break;
		}

		_count.store(index, atomic::Order::RELAXED);
	}

	void set_down(u64 count) {
		sys_assert(count < get(), "Attempted to call set_down with desired workers greater than or equal to current workers");

		const u64 current = get();

		// Signal the workers that are being removed.
		for (u64 index = count; index < current; ++index)
			_workers[index].terminate();

		// Wait for their threads to actually exit.
		for (u64 index = count; index < current; ++index)
			_workers[index].join();

		_count.store(count, atomic::Order::RELAXED);
	}

	void configure() {
		for (u64 index = 0; index < get(); ++index)
			_workers[index].configure(&_finished, _workers, get(), index);
	}

	void terminate() {
		for (u64 index = 0; index < get(); ++index) {
			_workers[index].terminate();
		}
	}

	void join() {
		for (u64 index = 0; index < get(); ++index) {
			_workers[index].join();
		}
	}

	void start() {
		for (u64 index = 0; index < get(); ++index)
			_workers[index].start();
	}

	void inject(const Job *jobs, u64 count) {
		for (u64 i = 0; i < count; ++i) {
			_workers[i % get()].inject(jobs[i], _root);
		}
	}

	void stop() {
		for (u64 i = 0; i < get(); ++i) {
			_workers[i].stop();
		}
		
		for (;;) {
			const u32 finished = _finished.word.load(atomic::Order::ACQUIRE);

			if (finished == get())
				break;

			_finished.wait(finished);
		}
	}

	void clear() {
		for (u64 index = 0; index < get(); ++index) {
			_workers[index].clear();
		}

		for (;;) {
			const u32 finished = _finished.word.load(atomic::Order::ACQUIRE);

			if (finished == get())
				break;

			_finished.wait(finished);
		}
	}

public:

	Pool() : _count(0), _finished(0), _active(false) {}

	~Pool() {
		terminate();
		join();
	}

	u64 set(u64 count) {
		mutex::Scope scope(_mutex);

		if (!_active) {
			if (count > MAX)
				count = MAX;

			const u64 current = get();
			if (count > current)
				set_up(count);
			else if (count < current)
				set_down(count);

			configure();
		}

		return get();
	}

	u64 get() {
		return _count.load(atomic::Order::RELAXED);
	}

	bool begin(const Job *jobs, u64 count) {
		mutex::Scope scope(_mutex);

		if (_active)
			return false;

		if (get() == 0)
			return false;

		_active = true;

		_root.init();

		_finished.word.store(0, atomic::Order::RELEASE);

		inject(jobs, count);

		_root.release();

		start();

		return true;
	}

	u32 poll() {
		return _root.poll();
	}

	bool end() {
		mutex::Scope scope(_mutex);

		if (!_active)
			return true;

		if (poll() != 0)
			return false;

		stop();

		clear();

		_active = false;

		return true;
	}

	Root &root() {
		return _root;
	}
};

}

#endif