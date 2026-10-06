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

#ifndef DESCENT_THREAD_FUTEX_HPP
#define DESCENT_THREAD_FUTEX_HPP

#include <descent/thread/atomic.hpp>
#include <descent/time.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

class Futex {
public:

	static constexpr u64 MAX_WAIT_ANY = 4;
	static constexpr u64 INVALID_INDEX = MAX_WAIT_ANY;

	struct Pair {
		const Futex &futex;
		u32 expected;

		Pair(const Futex &futex, u32 expected) : futex(futex), expected(expected) {}
	};

	template <u64 N>
	struct Any {
		static_assert(N <= MAX_WAIT_ANY, "Cannot create Futex::Any with more than MAX_WAIT_ANY elements");

		Pair pairs[N]{};
	};

private:

	Futex(Futex &&) = delete;
	Futex(const Futex &) = delete;
	Futex& operator=(Futex &&) = delete;
	Futex& operator=(const Futex &) = delete;

	static u64 wait_any(const Pair *pairs, u64 size, u64 timeout);

public:

	Atomic<u32> word;

	explicit Futex(u32 value = 0) : word(value) {}

	bool wait(u32 expected, u64 timeout = time::INDEFINITE_TIMEOUT) const;

	template <u64 N>
	static u64 wait_any(const Any<N> &any, u64 timeout = time::INDEFINITE_TIMEOUT) {
		return wait_any(any.pairs, N, timeout);
	}

	void wake_one() const;

	void wake_all() const;
};

}

#endif