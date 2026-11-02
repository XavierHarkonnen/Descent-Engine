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
#include <descent/type/utility.hpp>

namespace descent::thread {


template<typename R, typename... Args>
class Once {
	Once(Once &&)                  = delete;
	Once(const Once &)             = delete;
	Once &operator =(Once &&)      = delete;
	Once &operator =(const Once &) = delete;

private:
	enum {
		UNINITIALIZED = 0,
		RUNNING       = 1,
		INITIALIZED   = 2,
	};

	Futex futex;
	R (*function)(Args...);
	bool (*accept)(R) = nullptr;
	R result{};
	const R rejected{};

public:
	constexpr Once(R (*function)(Args...)) : futex(UNINITIALIZED), function(function) {
		if (!function)
			__builtin_trap();
	}

	constexpr Once(R (*function)(Args...), bool (*accept)(R), R init)
	: futex(UNINITIALIZED), function(function), accept(accept), rejected(move(init)) {
		if (!function || !accept)
			__builtin_trap();
	}

	const R &operator ()(Args... args) {
		if (futex.word.load(atomic::Order::ACQUIRE) == INITIALIZED)
			return result;

		for (;;) {
			u32 expected = UNINITIALIZED;
			// Must be a strong CAS: a spurious failure would leave expected == UNINITIALIZED
			// and the switch below would run function without owning the Once.
			futex.word.compare_exchange(expected, RUNNING, atomic::Order::ACQ_REL, atomic::Order::ACQUIRE);

			switch (expected) {
				case UNINITIALIZED: {
					R output = function(static_cast<Args &&>(args)...);

					if (!accept || accept(output)) {
						result = static_cast<R &&>(output);
						futex.word.store(INITIALIZED, atomic::Order::RELEASE);
						futex.wake_all();
						return result;
					}

					futex.word.store(UNINITIALIZED, atomic::Order::RELEASE);
					futex.wake_all();
					return rejected;
				}
				case RUNNING: {
					futex.wait(RUNNING);
					break;
				}
				case INITIALIZED: {
					return result;
				}
			}
		}
	}
};

template<typename... Args>
class Once<void, Args...> {
	Once(Once &&)                  = delete;
	Once(const Once &)             = delete;
	Once &operator =(Once &&)      = delete;
	Once &operator =(const Once &) = delete;

private:
	enum {
		UNINITIALIZED = 0,
		RUNNING       = 1,
		INITIALIZED   = 2,
	};

	Futex futex;
	void (*function)(Args...);

public:
	constexpr Once(void (*function)(Args...)) : futex(UNINITIALIZED), function(function) {
		if (!function)
			__builtin_trap();
	}

	void operator ()(Args... args) {
		if (futex.word.load(atomic::Order::ACQUIRE) == INITIALIZED)
			return;

		for (;;) {
			u32 expected = UNINITIALIZED;
			futex.word.compare_exchange(expected, RUNNING, atomic::Order::ACQ_REL, atomic::Order::ACQUIRE);

			switch (expected) {
				case UNINITIALIZED: {
					function(static_cast<Args &&>(args)...);
					futex.word.store(INITIALIZED, atomic::Order::RELEASE);
					futex.wake_all();
					return;
				}
				case RUNNING: {
					futex.wait(RUNNING);
					break;
				}
				case INITIALIZED: {
					return;
				}
			}
		}
	}
};

}

#endif
