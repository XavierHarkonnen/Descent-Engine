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

#include <time.h>

#include <descent/system.hpp>
#include <descent/type.hpp>
#include <intern/time.hpp>


namespace descent::time {

__attribute__((constructor))
static void init(void) {
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		sys_fatal("Platform does not support CLOCK_MONOTONIC");
	}
}

u64 now(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return intern::duration_from_timespec(ts);
}

u64 sleep(u64 duration) {
	if (duration == 0)
		return 0;

	struct timespec ts = intern::timespec_from_duration(duration);
	struct timespec rts;
	if (nanosleep(&ts, &rts))
		return intern::duration_from_timespec(rts);

	return 0;
}

}