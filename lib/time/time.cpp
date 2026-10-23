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

#include <descent/time.hpp>
 
extern "C" {
#include <errno.h>
#include <time.h>
}

#include <descent/system.hpp>
#include <descent/thread/utilities.hpp>
#include <descent/type/core.hpp>
#include <intern/time.hpp>

namespace descent::time {

__attribute__((constructor))
static void init(void) {
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		sys_fatal("Platform does not support CLOCK_MONOTONIC");
	}

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		sys_fatal("Platform does not support CLOCK_REALTIME");
	}
}

monotonic monotonic::now() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return monotonic(intern::time::ns_from_timespec(ts));
}

timestamp timestamp::now() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return timestamp(intern::time::ns_from_timespec(ts));
}

void sleep(duration d) {
	if (d.ns() <= 0)
		return;

	sleep(monotonic::now() + d);
}

void sleep(monotonic deadline) {
	timespec ts = intern::time::timespec_from_ns(deadline.ns());

	for (;;) {
		int rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr);
		if (rc == 0)
			return;

		if (rc == EINTR)
			continue;

		// Busy-wait on failure in production - undesirable but better than terminating.
		sys_trap("clock_nanosleep failed");
		thread::spin_pause();
	}
}

}