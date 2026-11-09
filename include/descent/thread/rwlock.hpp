/* Copyright 2026 XavierHarkonnen9 and Enlarium
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

#ifndef DESCENT_THREAD_RWLOCK_HPP
#define DESCENT_THREAD_RWLOCK_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/futex.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

class RWLock {
	RWLock(RWLock &&)                  = delete;
	RWLock(const RWLock &)             = delete;
	RWLock &operator =(RWLock &&)      = delete;
	RWLock &operator =(const RWLock &) = delete;

private:
	static constexpr u32 WRITE_FLAG  = 1u << 31;
	static constexpr u32 WAIT_FLAG   = 1u << 30;
	static constexpr u32 READER_MASK = WAIT_FLAG - 1;

	Futex futex;

	void sleep(u32 seen) {
		if (!(seen & WAIT_FLAG)) {
			if (!futex.word.compare_exchange(seen, seen | WAIT_FLAG, atomic::Order::RELAXED, atomic::Order::RELAXED))
				return;

			seen |= WAIT_FLAG;
		}

		futex.wait(seen);
	}

public:
	RWLock() : futex(0) {}

	void rlock() {
		for (;;) {
			u32 state = futex.word.load(atomic::Order::RELAXED);

			if (!(state & (WRITE_FLAG | WAIT_FLAG)) && (state & READER_MASK) != READER_MASK) {
				if (futex.word.compare_exchange(state, state + 1, atomic::Order::ACQUIRE, atomic::Order::RELAXED))
					return;

				continue;
			}

			sleep(state);
		}
	}

	void runlock() {
		const u32 state = futex.word.sub_fetch(1, atomic::Order::RELEASE);

		if ((state & READER_MASK) == 0 && (state & WAIT_FLAG)) {
			u32 expected = WAIT_FLAG;
			futex.word.compare_exchange(expected, 0, atomic::Order::RELAXED, atomic::Order::RELAXED);
			futex.wake_all();
		}
	}

	void wlock() {
		for (;;) {
			u32 state = futex.word.load(atomic::Order::RELAXED);

			if (!(state & (WRITE_FLAG | READER_MASK))) {
				if (futex.word.compare_exchange(
						state, WRITE_FLAG | (state & WAIT_FLAG), atomic::Order::ACQUIRE, atomic::Order::RELAXED
					))
					return;

				continue;
			}

			sleep(state);
		}
	}

	void wunlock() {
		const u32 state = futex.word.exchange(0, atomic::Order::RELEASE);

		if (state & WAIT_FLAG)
			futex.wake_all();
	}
};

namespace rwlock {

class RScope {
	RScope(RScope &&)                  = delete;
	RScope(const RScope &)             = delete;
	RScope &operator =(RScope &&)      = delete;
	RScope &operator =(const RScope &) = delete;

private:
	RWLock &_lock;

public:
	explicit RScope(RWLock &lock) : _lock(lock) {
		_lock.rlock();
	}

	~RScope() {
		_lock.runlock();
	}
};

class WScope {
	WScope(WScope &&)                  = delete;
	WScope(const WScope &)             = delete;
	WScope &operator =(WScope &&)      = delete;
	WScope &operator =(const WScope &) = delete;

private:
	RWLock &_lock;

public:
	explicit WScope(RWLock &lock) : _lock(lock) {
		_lock.wlock();
	}

	~WScope() {
		_lock.wunlock();
	}
};

}

}

#endif
