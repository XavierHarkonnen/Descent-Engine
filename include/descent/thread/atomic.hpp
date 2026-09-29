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

#ifndef DESCENT_THREAD_ATOMIC_HPP
#define DESCENT_THREAD_ATOMIC_HPP

namespace descent::thread {

namespace atomic {

enum class Order {
	RELAXED = __ATOMIC_RELAXED,
	ACQUIRE = __ATOMIC_ACQUIRE,
	RELEASE = __ATOMIC_RELEASE,
	ACQ_REL = __ATOMIC_ACQ_REL,
	SEQ_CST = __ATOMIC_SEQ_CST
};

class Flag {
private:
	bool _;

public:
	constexpr explicit Flag(bool value = false) : _(value) {}

	bool test_and_set(Order order) {
		return __atomic_test_and_set(&_, static_cast<int>(order));
	}

	void set(Order order) {
		__atomic_store_n(&_, true, static_cast<int>(order));
	}

	void clear(Order order) {
		__atomic_clear(&_, static_cast<int>(order));
	}

	bool load(Order order) {
		return __atomic_load_n(&_, static_cast<int>(order));
	}
};

static inline void compiler_barrier(Order order) {
	__atomic_signal_fence(static_cast<int>(order));
}

static inline void memory_barrier(Order order) {
	__atomic_thread_fence(static_cast<int>(order));
}

}

template <typename T>
class Atomic {
	friend class Futex;

	static_assert(__atomic_is_lock_free(sizeof(T), nullptr));

private:
	T _;

public:
	constexpr explicit Atomic(T value) : _(value) {}
	constexpr explicit Atomic() : _() {}

	T load(atomic::Order order) const {
		return __atomic_load_n(&_, static_cast<int>(order));
	}

	void store(T value, atomic::Order order) {
		__atomic_store_n(&_, value, static_cast<int>(order));
	}

	T exchange(T value, atomic::Order order) {
		return __atomic_exchange_n(&_, value, static_cast<int>(order));
	}
	
	bool compare_exchange(T &expected, T desired, atomic::Order success, atomic::Order failure) {
		return __atomic_compare_exchange_n(&_, &expected, desired, false, static_cast<int>(success), static_cast<int>(failure));
	}

	T add_fetch(T value, atomic::Order order) {
		return __atomic_add_fetch(&_, value, static_cast<int>(order));
	}

	T sub_fetch(T value, atomic::Order order) {
		return __atomic_sub_fetch(&_, value, static_cast<int>(order));
	}

	T and_fetch(T value, atomic::Order order) {
		return __atomic_and_fetch(&_, value, static_cast<int>(order));
	}

	T xor_fetch(T value, atomic::Order order) {
		return __atomic_xor_fetch(&_, value, static_cast<int>(order));
	}

	T or_fetch(T value, atomic::Order order) {
		return __atomic_or_fetch(&_, value, static_cast<int>(order));
	}

	T nand_fetch(T value, atomic::Order order) {
		return __atomic_nand_fetch(&_, value, static_cast<int>(order));
	}

	T fetch_add(T value, atomic::Order order) {
		return __atomic_fetch_add(&_, value, static_cast<int>(order));
	}

	T fetch_sub(T value, atomic::Order order) {
		return __atomic_fetch_sub(&_, value, static_cast<int>(order));
	}

	T fetch_and(T value, atomic::Order order) {
		return __atomic_fetch_and(&_, value, static_cast<int>(order));
	}

	T fetch_xor(T value, atomic::Order order) {
		return __atomic_fetch_xor(&_, value, static_cast<int>(order));
	}

	T fetch_or(T value, atomic::Order order) {
		return __atomic_fetch_or(&_, value, static_cast<int>(order));
	}

	T fetch_nand(T value, atomic::Order order) {
		return __atomic_fetch_nand(&_, value, static_cast<int>(order));
	}
};

}

#endif