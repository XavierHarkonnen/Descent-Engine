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

// thread::Semaphore: wait() and wait(time::duration), matching Mutex.

extern "C" {
#include <pthread.h>
#include <time.h>
#include <unistd.h>
}

#include <harness.hpp>

#include <descent/thread/atomic.hpp>
#include <descent/thread/semaphore.hpp>
#include <descent/time.hpp>

using namespace descent;
using namespace descent::thread;

static i64 now_ms() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return i64(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

static void wait_consumes() {
	Semaphore s(2);

	const i64 start      = now_ms();
	const bool a         = s.wait();
	const bool b         = s.wait();
	const i64 elapsed_ms = now_ms() - start;

	test_check(a && b, "wait() on count 2 failed (%s, %s)", a ? "ok" : "fail", b ? "ok" : "fail");
	test_check(elapsed_ms < 50, "wait() on an available count blocked %lld ms", (long long) elapsed_ms);
	test_check(!s.wait(time::duration::ms(0)), "a third wait(0) acquired after both counts were consumed");
}

static void timed_wait_expires() {
	Semaphore s(0);

	const i64 start      = now_ms();
	const bool acquired  = s.wait(time::duration::ms(100));
	const i64 elapsed_ms = now_ms() - start;

	test_check(!acquired, "wait(100ms) acquired with nothing posted");
	test_check(elapsed_ms >= 75, "wait(100ms) gave up after %lld ms", (long long) elapsed_ms);
}

static Semaphore posted(0);

static void *post_later(void *) {
	usleep(50000);
	posted.post();
	return nullptr;
}

static void timed_wait_acquires() {
	pthread_t t;
	test_require(pthread_create(&t, nullptr, post_later, nullptr) == 0, "pthread_create");

	const i64 start      = now_ms();
	const bool acquired  = posted.wait(time::duration::ms(1000));
	const i64 elapsed_ms = now_ms() - start;

	pthread_join(t, nullptr);

	test_check(acquired, "wait(1000ms) timed out although a count was posted after 50 ms");
	test_check(elapsed_ms < 900, "wait(1000ms) acquired after %lld ms", (long long) elapsed_ms);
}

// wait(0) is a non-blocking try.
static void zero_timeout_try() {
	Semaphore s(1);

	test_check(s.wait(time::duration::ms(0)), "wait(0) with count 1 did not acquire");

	const i64 start      = now_ms();
	const bool second    = s.wait(time::duration::ms(0));
	const i64 elapsed_ms = now_ms() - start;

	test_check(!second, "wait(0) with count 0 acquired");
	test_check(elapsed_ms < 10, "wait(0) with count 0 blocked %lld ms", (long long) elapsed_ms);
}

// Every post is consumed exactly once and no waiter is left asleep; a lost
// wakeup shows up as a timeout.

static constexpr u32 PRODUCERS  = 4;
static constexpr u32 CONSUMERS  = 4;
static constexpr u32 PER_THREAD = 20000;

static Semaphore items(0);
static Atomic<u64> consumed{ 0 };

static void *producer(void *) {
	for (u32 i = 0; i < PER_THREAD; ++i)
		items.post();
	return nullptr;
}

static void *consumer(void *) {
	for (u32 i = 0; i < PER_THREAD; ++i) {
		items.wait();
		consumed.fetch_add(1, atomic::Order::RELAXED);
	}
	return nullptr;
}

static void producer_consumer() {
	pthread_t threads[PRODUCERS + CONSUMERS];

	// Consumers first, so most of them block before posts arrive.
	for (u32 i = 0; i < CONSUMERS; ++i)
		test_require(pthread_create(&threads[i], nullptr, consumer, nullptr) == 0, "pthread_create");
	for (u32 i = 0; i < PRODUCERS; ++i)
		test_require(pthread_create(&threads[CONSUMERS + i], nullptr, producer, nullptr) == 0, "pthread_create");
	for (pthread_t &t : threads)
		pthread_join(t, nullptr);

	const u64 total = consumed.load(atomic::Order::RELAXED);
	test_check(
		total == u64(PRODUCERS) * PER_THREAD, "consumed %llu of %llu", (unsigned long long) total,
		(unsigned long long) PRODUCERS * PER_THREAD
	);
	test_check(!items.wait(time::duration::ms(0)), "semaphore not empty after every post was consumed");
}

static constexpr test::Case cases[] = {
	{ "wait_consumes", wait_consumes, 5 },
	{ "timed_wait_expires", timed_wait_expires, 5 },
	{ "timed_wait_acquires", timed_wait_acquires, 5 },
	{ "zero_timeout_try", zero_timeout_try, 5 },
	{ "producer_consumer", producer_consumer, 20 },
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
