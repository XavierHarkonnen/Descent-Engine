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

#include <descent/bsl/level.hpp>
#include <descent/bsl/site.hpp>
#include <descent/random.hpp>
#include <descent/thread/once.hpp>
#include <descent/time.hpp>

namespace descent::bsl {

constexpr u64 SESSION_ID_SIZE = 16;
constexpr u64 BSL_PATH_LENGTH = SESSION_ID_SIZE + 12;

static thread_local Level _level = LEVEL;

void enable(descent::bsl::Level level) {
	_level |= level;
}

void disable(descent::bsl::Level level) {
	_level &= ~level;
}

Level enabled() {
	return LEVEL | _level;
}

// This session ID doesn't need to be particularly resilient, as it can be rerolled until success
// Use openat/mkdirat for logging-dir
// <logging-dir>/<session-timestamp>/symbol.bsl
// <logging-dir>/<session-timestamp>/<thread-id>.bsl

#define LOG_DIR "./descent-engine/"

static int generate_session_directory() {
	i64 timestamp = time::timestamp::now().ns();

	char log_dir[] = LOG_DIR "00000000/";
}

// Return the file handle of the session ID directory
static int session_directory() {
	thread::Once once(generate_session_directory);
	return once();
}

static const char *symbol_path() {
	char path[sizeof(i64)];



	return path;
}

static const char *thread_path() {

}

template<typename T>
static void hexcpy(char *output, T input, u64 size) {
	static_assert(type::traits<T>::is_integer, "hexcpy requires integer argument");

	constexpr char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	using U = typename type::traits<T>::to_unsigned;
	U bits = U(input);

	for (u64 i = 0; i < size; ++i) {
		output[size - i - 1] = HEX[bits & 0xF];
		bits >>= 4;
	}
}

static const char *generate_symbol_file_path(u64 session_id) {
	static char symbol_file_path[BSL_PATH_LENGTH] = "0000000000000000/symbol.bsl";
	hexcpy(symbol_file_path, session_id, SESSION_ID_SIZE);
	return symbol_file_path;
}

static const char *generate_thread_file_path(u64 session_id, u64 thread_id) {
	if (thread_id > 0xFFFFFF)
		return nullptr;

	static thread_local char symbol_file_path[BSL_PATH_LENGTH] = "0000000000000000/t00000.bsl";
	hexcpy(symbol_file_path, session_id, SESSION_ID_SIZE);
	hexcpy(symbol_file_path + 18, thread_id, 5);
	return symbol_file_path;
}

static bool generate_symbol_file() {

	// Define symbol file name from timestamp
	// Create symbol file

	return true;
}

static bool generate_thread_file() {
	// Define thread file name from timestamp
	// Get site count
	// Create log file

	return true;
}

static bool init_symbol() {
	static thread::Once once(generate_symbol_file);
	return once();
}

bool init_thread() {
	if (!init_symbol())
		return false;

	static thread::Once once(generate_thread_file);
	return once();
}

}
