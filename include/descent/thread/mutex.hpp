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

#ifndef DESCENT_THREAD_MUTEX_HPP
#define DESCENT_THREAD_MUTEX_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/futex.hpp>
#include <descent/thread/utilities.hpp>
#include <descent/time.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

class Mutex {
	Mutex(Mutex &&) = delete;
	Mutex(const Mutex &) = delete;
	Mutex operator =(Mutex &&) = delete;
	Mutex operator =(const Mutex &) = delete;

private:
	static constexpr u64 MUTEX_SPIN_COUNT = 64;

	enum {
		FREE      = 0,
		LOCKED    = 1,
		CONTENDED = 2,
	};

	Futex futex;

public:
	Mutex() : futex(FREE) {}

	bool trylock() {
		u32 expected = FREE;
		return futex.word.compare_exchange(expected, LOCKED, atomic::Order::ACQUIRE, atomic::Order::RELAXED);
	}

	bool lock() {
		if (trylock())
			return true;

		u64 spins = MUTEX_SPIN_COUNT;
		while (spins-- && futex.word.load(atomic::Order::RELAXED) == LOCKED)
			spin_pause();

		u32 state = FREE;
		if (futex.word.compare_exchange(state, LOCKED, atomic::Order::ACQUIRE, atomic::Order::RELAXED))
			return true;

		// A thread that may sleep must acquire as CONTENDED, since it cannot
		// know whether other waiters remain and its unlock must then wake one.
		if (state != CONTENDED)
			state = futex.word.exchange(CONTENDED, atomic::Order::ACQUIRE);

		while (state != FREE) {
			futex.wait(CONTENDED);
			state = futex.word.exchange(CONTENDED, atomic::Order::ACQUIRE);
		}

		return true;
	}

	bool lock(time::duration timeout) {
		if (trylock())
			return true;

		time::monotonic start = time::monotonic::now();

		u64 spins = MUTEX_SPIN_COUNT;
		while (spins-- && futex.word.load(atomic::Order::RELAXED) == LOCKED)
			spin_pause();

		u32 state = FREE;
		if (futex.word.compare_exchange(state, LOCKED, atomic::Order::ACQUIRE, atomic::Order::RELAXED))
			return true;

		// See lock(). Timing out leaves the word CONTENDED, which only costs
		// the holder one unnecessary wake.
		if (state != CONTENDED)
			state = futex.word.exchange(CONTENDED, atomic::Order::ACQUIRE);

		while (state != FREE) {
			time::monotonic now = time::monotonic::now();
			time::duration elapsed = now - start;
			if (elapsed >= timeout)
				return false;

			time::duration remaining = timeout - elapsed;

			futex.wait(CONTENDED, remaining);
			state = futex.word.exchange(CONTENDED, atomic::Order::ACQUIRE);
		}

		return true;
	}

	void unlock() {
		u32 state = futex.word.exchange(FREE, atomic::Order::RELEASE);

		if (state == CONTENDED)
			futex.wake_one();
	}
};

namespace mutex {
class Scope {
	Scope(Scope &&) = delete;
	Scope(const Scope &) = delete;
	Scope &operator=(Scope &&) = delete;
	Scope &operator=(const Scope &) = delete;

private:
	Mutex &_mutex;

public:
	explicit Scope(Mutex &mutex) : _mutex(mutex) {
		_mutex.lock();
	}

	~Scope() {
		_mutex.unlock();
	}
};
}

}

#endif
