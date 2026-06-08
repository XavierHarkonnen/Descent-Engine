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

#include <time.h>

#include <descent/types/core.h>

#include "time.h"

u64 time_now(void) {
	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return duration_from_timespec(ts);
}

u64 time_sleep(u64 duration) {
	struct timespec ts = timespec_from_duration(duration);
	struct timespec rts;
	if (nanosleep(&ts, &rts))
		return duration_from_timespec(rts);

	return 0;
}