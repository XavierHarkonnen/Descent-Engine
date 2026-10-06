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

#ifndef DESCENT_THREAD_THREAD_HPP
#define DESCENT_THREAD_THREAD_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

template<class T>
struct RemoveReference {
	using Type = T;
};

template<class T>
struct RemoveReference<T&> {
	using Type = T;
};

template<class T>
struct RemoveReference<T&&> {
	using Type = T;
};

template<class T>
constexpr typename RemoveReference<T>::Type&& move(T&& arg) noexcept {
	return static_cast<typename RemoveReference<T>::Type&&>(arg);
}

using Routine = void (*)(void *);

// Non-copyable handle to a managed thread.
//
// Thread creation is restricted to the main thread.
//
// A valid handle does not imply that the underlying thread is still executing.
// Finished threads continue to occupy a managed slot until joined or collected.
//
// Destroying a handle does not terminate or join its thread. Instead, the
// thread becomes eligible for collection.
//
// Handles may be transferred using move operations.
class Thread {
	Thread(const Thread &) = delete;
	Thread &operator=(const Thread &) = delete;

private:
	static constexpr u32 INVALID = ~u32(0);

	Atomic<u32> _;
	Mutex lock;

public:
	// Creates an invalid handle.
	Thread() : _(INVALID) {}

	// Creates a managed thread using the default stack size.
	// The resulting handle is invalid if creation fails.
	Thread(Routine routine, void *argument);

	// Creates a managed thread using the requested stack size.
	// Sizes below the platform minimum are raised to that minimum.
	// The resulting handle is invalid if creation fails.
	Thread(Routine routine, void *argument, u32 stack_size);

	// Transfers the handle from `other`, invalidating `other`.
	Thread(Thread &&other) {
		mutex::Scope scope(other.lock);

		u32 index = other._.load(atomic::Order::RELAXED);
		_.store(index, atomic::Order::RELAXED);
		other._.store(INVALID, atomic::Order::RELAXED);
	};

	~Thread();

	// Releases the current handle's ownership, if any, then transfers
	// the handle from other, invalidating other.
	Thread &operator=(Thread &&other);

	// Returns whether this handle refers to a managed thread.
	// Does not indicate whether the thread is still running.
	explicit operator bool() const {
		return _.load(atomic::Order::RELAXED) != INVALID;
	}

	// Waits for the thread to terminate and releases its managed slot.
	// Only available to the main thread.
	// Returns false if called from another thread or with an invalid handle.
	// Invalidates the handle if successful.
	bool join();

	// Adds signal bits to the thread's pending signal set.
	// Does not explicitly wake a thread blocked in wait().
	// Returns false if the handle is invalid.
	bool send(u32 signals);

	// Adds signal bits and wakes one thread waiting for signals.
	// Returns false if the handle is invalid.
	bool wake(u32 signals);

	// Atomically reads and clears pending signals for the calling thread.
	// Returns zero if no signals are pending or the caller is not managed.
	static u32 poll();

	// Waits until pending signals are available, then reads and clears them.
	// Returns zero immediately if the caller is not managed.
	static u32 wait();

	// Joins and reclaims threads without an owning handle.
	// Only performs collection on the main thread.
	// May block while waiting for orphaned threads to terminate.
	// Returns the number of allocated slots remaining after collection.
	static u32 collect();

	// Returns the maximum number of managed thread slots.
	static u32 maximum();
};

}

#endif