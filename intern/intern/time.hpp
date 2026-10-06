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

namespace descent::intern {

static inline u64 duration_from_timespec(struct timespec ts) {
	return (u64) ts.tv_nsec + (u64) ts.tv_sec * time::NANOSECONDS_PER_SECOND;
}

static inline struct timespec timespec_from_duration(u64 duration) {
	struct timespec ts;
	ts.tv_sec = (time_t) (duration / time::NANOSECONDS_PER_SECOND);
	ts.tv_nsec = (long) (duration % time::NANOSECONDS_PER_SECOND);
	return ts;
}

}

#endif