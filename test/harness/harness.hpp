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

// Shared test harness.
//
// A test is a table of cases passed to test::run(). Each case runs in its own
// child process with a wall-clock limit, so a crash or hang fails that case
// alone and is reported by name. The child's stdout and stderr are captured and
// shown only when the case fails.
//
// Checks work in every build type, unlike sys_assert, which compiles out of
// release builds:
//
//     test_check(expression, format, ...)    records a failure and continues
//     test_require(expression, format, ...)  records a failure and ends the case
//
// A case can instead expect a trap: it passes only if the child exits with a
// failure status and its output contains the given text, for example the
// message of a debug sys_assert.
//
// Running a test executable with case names as arguments runs only those cases.

#ifndef DESCENT_TEST_HARNESS_HARNESS_HPP
#define DESCENT_TEST_HARNESS_HARNESS_HPP

extern "C" {
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
}

#include <descent/type/core.hpp>

namespace descent::test {

struct Case {
	const char *name;
	void (*function)();
	u32 timeout = 10;          // Seconds
	const char *trap = nullptr; // If set, the case must trap with this text
};

// Failed checks in the current case. Updated atomically, since checks may run
// on threads the case starts.
inline u32 failures = 0;

[[gnu::format(printf, 4, 5)]]
inline void fail(const char *expression, const char *file, u32 line, const char *format, ...) {
	__atomic_fetch_add(&failures, 1, __ATOMIC_RELAXED);

	// Captured stdout is fully buffered; flush it so it stays in order with
	// this message.
	fflush(stdout);
	fprintf(stderr, "%s:%u: check '%s' failed: ", file, line, expression);

	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fputc('\n', stderr);
}

[[noreturn]]
inline void end_case() {
	fflush(stdout);
	fflush(stderr);
	_exit(__atomic_load_n(&failures, __ATOMIC_RELAXED) ? 1 : 0);
}

namespace detail {

constexpr u64 OUTPUT_CAPACITY = 64 * 1024;

enum class Outcome : u8 { PASSED, FAILED, CRASHED, TIMED_OUT, NO_TRAP };

struct Result {
	Outcome outcome;
	int detail;       // Exit code or signal number
	u64 elapsed_ms;
	u64 length;       // Captured output length
	bool truncated;
};

inline u64 now_ms() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return u64(ts.tv_sec) * 1000 + u64(ts.tv_nsec) / 1000000;
}

inline bool selected(const char *name, int argc, char **argv) {
	if (argc < 2)
		return true;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(name, argv[i]) == 0)
			return true;
	}

	return false;
}

inline Result run_case(const Case &c, char *output) {
	fflush(stdout);
	fflush(stderr);

	int fds[2];
	if (pipe(fds) != 0) {
		perror("test: pipe");
		_exit(2);
	}

	const u64 start = now_ms();

	const pid_t pid = fork();
	if (pid < 0) {
		perror("test: fork");
		_exit(2);
	}

	if (pid == 0) {
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		dup2(fds[1], STDERR_FILENO);
		close(fds[1]);

		alarm(c.timeout);
		c.function();
		end_case();
	}

	close(fds[1]);

	// Read until every writer has exited, keeping what fits.
	Result r{};
	char discard[4096];
	for (;;) {
		char *into = r.length < OUTPUT_CAPACITY - 1 ? output + r.length : discard;
		const u64 room = r.length < OUTPUT_CAPACITY - 1 ? OUTPUT_CAPACITY - 1 - r.length : sizeof(discard);

		const ssize_t n = read(fds[0], into, room);
		if (n <= 0)
			break;

		if (into == discard)
			r.truncated = true;
		else
			r.length += u64(n);
	}
	output[r.length] = '\0';
	close(fds[0]);

	int status = 0;
	waitpid(pid, &status, 0);
	r.elapsed_ms = now_ms() - start;

	if (WIFSIGNALED(status)) {
		r.detail = WTERMSIG(status);
		r.outcome = r.detail == SIGALRM ? Outcome::TIMED_OUT : Outcome::CRASHED;
	} else {
		r.detail = WEXITSTATUS(status);
		if (c.trap)
			r.outcome = r.detail != 0 && strstr(output, c.trap) ? Outcome::PASSED : Outcome::NO_TRAP;
		else
			r.outcome = r.detail == 0 ? Outcome::PASSED : Outcome::FAILED;
	}

	return r;
}

inline void report(const Case &c, const Result &r, const char *output) {
	switch (r.outcome) {
	case Outcome::PASSED:
		printf("[PASS] %s (%llu ms)\n", c.name, (unsigned long long) r.elapsed_ms);
		return;
	case Outcome::FAILED:
		printf("[FAIL] %s: exited with status %d\n", c.name, r.detail);
		break;
	case Outcome::CRASHED:
		printf("[FAIL] %s: killed by signal %d (%s)\n", c.name, r.detail, strsignal(r.detail));
		break;
	case Outcome::TIMED_OUT:
		printf("[FAIL] %s: timed out after %u s\n", c.name, c.timeout);
		break;
	case Outcome::NO_TRAP:
		printf("[FAIL] %s: expected a trap containing \"%s\"; exited with status %d\n", c.name, c.trap, r.detail);
		break;
	}

	if (r.length) {
		printf("------ output of %s\n%s", c.name, output);
		if (output[r.length - 1] != '\n')
			putchar('\n');
		if (r.truncated)
			printf("[output truncated]\n");
		printf("------\n");
	}
}

}

// Runs every selected case and returns the process exit status: 0 if all
// passed, 1 otherwise.
template<u64 N>
int run(const Case (&cases)[N], int argc, char **argv) {
	static char output[detail::OUTPUT_CAPACITY];

	u32 ran = 0;
	u32 failed = 0;

	for (u64 i = 0; i < N; ++i) {
		if (!detail::selected(cases[i].name, argc, argv))
			continue;

		const detail::Result r = detail::run_case(cases[i], output);
		detail::report(cases[i], r, output);

		++ran;
		if (r.outcome != detail::Outcome::PASSED)
			++failed;
	}

	printf("%u of %u case(s) passed\n", ran - failed, ran);
	fflush(stdout);

	return ran && !failed ? 0 : 1;
}

}

#define test_check(expression, ...)                                                      \
	((expression) ? (void) 0                                                             \
		: ::descent::test::fail(#expression, __FILE__, __LINE__, __VA_ARGS__))

#define test_require(expression, ...)                                                    \
	((expression) ? (void) 0                                                             \
		: (::descent::test::fail(#expression, __FILE__, __LINE__, __VA_ARGS__),          \
			::descent::test::end_case()))

#endif
