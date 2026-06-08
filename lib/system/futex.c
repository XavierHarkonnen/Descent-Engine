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

#define _GNU_SOURCE

#include <descent/system.h>

#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include <descent/types/core.h>

#include "time.h"

bool futex_wait(u32 *futex, u32 expected, u64 duration) {
	if (!futex) return false;

	struct timespec ts = timespec_from_duration(duration);
	struct timespec *tsp = &ts;
	if (duration != 0)
		tsp = NULL;

	long result = syscall(SYS_futex, futex, FUTEX_WAIT_PRIVATE, expected, tsp, NULL, 0);
	if (result == 0) return true;

	switch(errno) {
		case EINTR:  return true;
		case EAGAIN: return true;
		default:     return false;
	}
}

bool futex_wake_single(u32 *futex) {
	if (!futex) return false;
	long result = syscall(SYS_futex, futex, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
	return (result >= 0);
}

bool futex_wake_all(u32 *futex) {
	if (!futex) return false;
	long result = syscall(SYS_futex, futex, FUTEX_WAKE_PRIVATE, U32_MAX, NULL, NULL, 0);
	return (result >= 0);
}
