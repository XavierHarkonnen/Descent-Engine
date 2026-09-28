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
#include <descent/type.hpp>

namespace descent::thread {

class Mutex {
private:
	static constexpr u64 MUTEX_SPIN_COUNT = 64;

	enum {
		FREE      = 0,
		LOCKED    = 1,
		CONTENDED = 2,
	};

	Futex futex;

public:

	bool trylock() {
		u32 expected = FREE;
		return futex.word.compare_exchange(expected, LOCKED, atomic::Order::ACQUIRE, atomic::Order::RELAXED);
	}

	bool lock(u64 timeout = time::INDEFINITE_TIMEOUT) {
		if (trylock())
			return true;

		u64 start = time::now();

		u64 spins = MUTEX_SPIN_COUNT;
		while (spins-- && futex.word.load(atomic::Order::RELAXED) == LOCKED)
			spin_pause();

		for (;;) {
			u32 expected = FREE;
			if (futex.word.compare_exchange(expected, LOCKED, atomic::Order::ACQUIRE, atomic::Order::RELAXED))
				return true;

			if (expected == LOCKED)
				futex.word.compare_exchange(expected, CONTENDED, atomic::Order::RELAXED, atomic::Order::RELAXED);

			u64 now = time::now();
			u64 elapsed = now - start;
			if (timeout && elapsed >= timeout)
				return false;

			u64 remaining = timeout ? timeout - elapsed : 0;

			expected = CONTENDED;
			futex.wait(expected, remaining);
		}
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
