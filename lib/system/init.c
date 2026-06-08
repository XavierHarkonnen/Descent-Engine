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

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <descent/types/core.h>

static pid_t main_thread;

__attribute__((constructor))
static void init (void) {
	main_thread = gettid();

	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		const char message[] = "ERROR: Platform does not support CLOCK_MONOTONIC";
		write(STDERR_FILENO, message, sizeof(message) - 1);
		_Exit(EXIT_FAILURE);
	}
}

bool is_main_thread(void) {
	return gettid() == main_thread;
}