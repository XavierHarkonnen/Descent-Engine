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

#include <descent/thread/futex.hpp>

extern "C" {
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
}

#include <descent/time.hpp>
#include <descent/type.hpp>
#include <descent/system.hpp>

#include <intern/time.hpp>

namespace descent::thread {

bool Futex::wait(u32 expected, u64 timeout) const {
	struct timespec ts = intern::timespec_from_duration(timeout);
	struct timespec *tsp = &ts;
	
	if (timeout == time::INDEFINITE_TIMEOUT)
		tsp = nullptr;

	long result = syscall(SYS_futex, &word._, FUTEX_WAIT_PRIVATE, expected, tsp, NULL, 0);
	if (result == 0)
		return true;
		// Woken, should check predicate

	switch(errno) {
		case EINTR:
			return true;
			// Interrupted, should check predicate

		case EAGAIN:
			return true;
			// Value changed, should check predicate

		case ETIMEDOUT:
			return false;
			// Timeout, need not check predicate

		default:
			sys_trap("Futex wait failed");
			return true;
			// Unhandled failure, should check predicate
			// Failure to wait a thread is recoverable.
			// This event is either exceptionally rare or indicates programmer error.
			// In release, may cause a busywait. In debug, knowing about it is more important.
	}
}

u64 Futex::wait_any(const Pair *pairs, u64 size, u64 timeout) {
	sys_assert(size <= MAX_WAIT_ANY, "Futex wait_any size exceeds maximum");
	
	struct timespec ts = intern::timespec_from_duration(timeout);
	struct timespec *tsp = &ts;
	
	if (timeout == time::INDEFINITE_TIMEOUT)
		tsp = nullptr;

	u64 indices[MAX_WAIT_ANY];
	struct futex_waitv waitv[MAX_WAIT_ANY];
	u64 count = 0;

	for (u64 index = 0; index < size; ++index) {
		waitv[count].uaddr = reinterpret_cast<u64>(&pairs[index].futex.word._);
		waitv[count].val = pairs[index].expected;
		waitv[count].flags = FUTEX2_SIZE_U32 | FUTEX2_PRIVATE;
		waitv[count].__reserved = 0;
		indices[count] = index;

		++count;
	}

	if (count == 0) {
		return INVALID_INDEX;
	}

	long result = syscall(SYS_futex_waitv, waitv, count, 0, tsp, CLOCK_MONOTONIC);
	if (result >= 0)
		return indices[result];
		// Woken, should check predicate

	switch(errno) {
		case EINTR:
			return INVALID_INDEX;
			// Interrupted, should check predicate

		case EAGAIN:
			return INVALID_INDEX;
			// Value changed, should check predicate

		case ETIMEDOUT:
			return INVALID_INDEX;
			// Timeout, need not check predicate

		default:
			sys_trap("Futex wait failed");
			return INVALID_INDEX;
			// Unhandled failure, should check predicate
			// Failure to wait a thread is recoverable.
			// This event is either exceptionally rare or indicates programmer error.
			// In release, may cause a busywait. In debug, knowing about it is more important.
	}
}

void Futex::wake_one() const {
	if (syscall(SYS_futex, &word._, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0) < 0)
		sys_fatal("Futex wake failed");
		// Failure to wake a thread is unrecoverable.
		// This event is either exceptionally rare or indicates programmer error.
}

void Futex::wake_all() const {
	if (syscall(SYS_futex, &word._, FUTEX_WAKE_PRIVATE, I32_MAX, NULL, NULL, 0) < 0)
		sys_fatal("Futex wake failed");
		// Failure to wake a thread is unrecoverable.
		// This event is either exceptionally rare or indicates programmer error.
}

}
