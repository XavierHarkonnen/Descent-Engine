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

#ifndef DESCENT_INTERN_TIME_HPP
#define DESCENT_INTERN_TIME_HPP

extern "C" {
#include <time.h>
}

#include <descent/time.hpp>
#include <descent/type/core.hpp>

namespace descent::intern::time {

static constexpr i64 NANOSECONDS_PER_SECOND = 1000000000;

static inline i64 ns_from_timespec(struct timespec ts) {
	return (i64) ts.tv_nsec + (i64) ts.tv_sec * NANOSECONDS_PER_SECOND;
}

static inline struct timespec timespec_from_ns(i64 ns) {
	struct timespec ts;
	ts.tv_sec = (time_t) (ns / NANOSECONDS_PER_SECOND);
	ts.tv_nsec = (long) (ns % NANOSECONDS_PER_SECOND);

	if (ts.tv_nsec < 0) {
		ts.tv_sec -= 1;
		ts.tv_nsec += NANOSECONDS_PER_SECOND;
	}

	return ts;
}

}

#endif