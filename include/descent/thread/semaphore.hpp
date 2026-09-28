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
#include <descent/type.hpp>

namespace descent::thread {

class Semaphore {
private:

	Futex futex;

public:

	bool wait(u64 timeout = time::INDEFINITE_TIMEOUT) {
		u64 deadline = time::now() + timeout;

		for (;;) {
			u32 count = futex.word.load(atomic::Order::RELAXED);

			while (count != 0) {
				if (futex.word.compare_exchange(count, count - 1, atomic::Order::ACQUIRE, atomic::Order::RELAXED))
					return true;
			}

			u64 remaining = deadline - time::now();

			if (remaining <= 0)
				return false;

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
