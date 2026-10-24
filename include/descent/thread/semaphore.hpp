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

#ifndef DESCENT_THREAD_SEMAPHORE_HPP
#define DESCENT_THREAD_SEMAPHORE_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/futex.hpp>
#include <descent/thread/utilities.hpp>
#include <descent/time.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

class Semaphore {
	Semaphore(Semaphore &&)                 = delete;
	Semaphore(const Semaphore &)            = delete;
	Semaphore operator =(Semaphore &&)      = delete;
	Semaphore operator =(const Semaphore &) = delete;

private:

	Futex futex;

	// Takes one count if any are available.
	bool take() {
		u32 count = futex.word.load(atomic::Order::RELAXED);

		while (count != 0) {
			if (futex.word.compare_exchange(count, count - 1, atomic::Order::ACQUIRE, atomic::Order::RELAXED))
				return true;
		}

		return false;
	}

public:
	Semaphore(u32 count) : futex(count) {}

	bool wait() {
		for (;;) {
			if (take())
				return true;

			futex.wait(0);
		}
	}

	bool wait(time::duration timeout) {
		time::monotonic start = time::monotonic::now();

		for (;;) {
			if (take())
				return true;

			time::monotonic now    = time::monotonic::now();
			time::duration elapsed = now - start;
			if (elapsed >= timeout)
				return false;

			time::duration remaining = timeout - elapsed;

			futex.wait(0, remaining);
		}
	}

	void post() {
		futex.word.fetch_add(1, atomic::Order::RELEASE);
		futex.wake_one();
	}
};

}

#endif
