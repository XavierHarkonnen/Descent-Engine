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

#ifndef DESCENT_THREAD_ONCE_HPP
#define DESCENT_THREAD_ONCE_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/futex.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

template<typename R, typename ...Args>
class Once {
	Once(Once &&) = delete;
	Once(const Once &) = delete;
	Once &operator =(Once &&) = delete;
	Once &operator =(const Once &) = delete;

private:
	static constexpr u64 ONCE_SPIN_COUNT = 64;

	enum {
		UNINITIALIZED = 0,
		RUNNING       = 1,
		INITIALIZED   = 2,
	};

	Futex futex;
	R (*function)(Args...);
	R result;

public:
	Once(R (*function)(Args...)) : futex(UNINITIALIZED), function(function) {}

	const R& operator ()(Args... args) {
		if (futex.word.load(atomic::Order::ACQUIRE) == INITIALIZED)
			return result;

		for (;;) {
			u32 expected = UNINITIALIZED;
			futex.word.compare_exchange(expected, RUNNING, atomic::Order::ACQ_REL, atomic::Order::ACQUIRE);

			switch (expected) {
				case UNINITIALIZED:
					result = function(static_cast<Args &&>(args)...);
					futex.word.store(INITIALIZED, atomic::Order::RELEASE);
					futex.wake_all();
					return result;
				case RUNNING:
					futex.wait(RUNNING);
					break;
				case INITIALIZED:
					return result;
			}
		}
	}
};

}

#endif
