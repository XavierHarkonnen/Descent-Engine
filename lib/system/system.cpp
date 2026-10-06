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

#include <descent/system.hpp>

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
}

static inline void sys_err_write(const char *s) {
	(void) write(STDERR_FILENO, s, strnlen(s, 1023));
}

#undef sys_fatal
__attribute__((cold, noreturn))
void sys_fatal(const char *message, const char *file, const char *line) {
	sys_err_write(file);
	sys_err_write(":");
	sys_err_write(line);
	sys_err_write(": fatal error: ");
	sys_err_write(message ? message : "<no message>");
	sys_err_write("\n");

	_Exit(EXIT_FAILURE);
}

#undef sys_assert
#if defined(DESCENT_DEBUG)
__attribute__((cold, noreturn))
void sys_assert(const char *expression, const char *message, const char *file, const char *line) {
	sys_err_write(file);
	sys_err_write(":");
	sys_err_write(line);
	sys_err_write(": assertion '");
	sys_err_write(expression);
	sys_err_write("' failed: ");
	sys_err_write(message ? message : "<no message>");
	sys_err_write("\n");

	_Exit(EXIT_FAILURE);
}
#endif
