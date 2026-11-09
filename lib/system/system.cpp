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
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
}

#include <descent/type/core.hpp>

using namespace descent;

static constexpr int MAX_SYS_MESSAGE_SIZE = 1024;

static void sys_copy(char *destination, u64 &offset, u64 capacity, const char *string) {
	if (offset >= capacity)
		return;

	u64 i = 0;

	while ((offset < capacity - 1) && string[i])
		destination[offset++] = string[i++];
	
	destination[offset] = '\0';
}

static void sys_write(const char *buffer, u64 length) {
	while (length) {
		ssize_t written = write(STDERR_FILENO, buffer, length);

		if (written > 0) {
			buffer += written;
			length -= u64(written);
			continue;
		}

		if (written < 0 && errno == EINTR)
			continue;

		break;
	}
}

#undef sys_fatal
[[noreturn, gnu::cold]]
void sys_fatal(const char *message, const char *file, const char *line) {
	char buffer[MAX_SYS_MESSAGE_SIZE];

	if (!message)
		message = "?";
	if (!file)
		file = "?";
	if (!line)
		line = "?";

	u64 offset = 0;
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, file);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, ":");
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, line);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, " FATAL: ");
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, message);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, "\n");

	(void) sys_write(buffer, offset);

	_Exit(EXIT_FAILURE);
}

#undef sys_assert
[[noreturn, gnu::cold]]
void sys_assert(const char *expression, const char *message, const char *file, const char *line) {
	char buffer[MAX_SYS_MESSAGE_SIZE];

	if (!expression)
		expression = "?";
	if (!message)
		message = "?";
	if (!file)
		file = "?";
	if (!line)
		line = "?";

	u64 offset = 0;
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, file);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, ":");
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, line);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, " ASSERTION '");
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, expression);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, "' FAILED: ");
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, message);
	sys_copy(buffer, offset, MAX_SYS_MESSAGE_SIZE, "\n");

	(void) sys_write(buffer, offset);

	_Exit(EXIT_FAILURE);
}
