// thread::Futex: timed waits on one futex and on several (wait_any).
//
// futex_waitv(2) takes an absolute CLOCK_MONOTONIC timeout, while Futex takes
// a relative duration, so wait_any must convert it (fixed in db962ae).

extern "C" {
#include <pthread.h>
#include <time.h>
#include <unistd.h>
}

#include <harness.hpp>

#include <descent/thread/futex.hpp>
#include <descent/time.hpp>

using namespace descent;
using namespace descent::thread;

static constexpr i64 TIMEOUT_MS = 200;
// Lower bound with generous scheduling slack.
static constexpr i64 MIN_MS = TIMEOUT_MS * 3 / 4;

static i64 now_ms() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return i64(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

static void wait_timeout() {
	Futex a(0);

	const i64 start = now_ms();
	a.wait(0, time::duration::ms(TIMEOUT_MS));
	const i64 elapsed_ms = now_ms() - start;

	test_check(elapsed_ms >= MIN_MS, "wait(0, %lldms) returned after %lld ms", (long long) TIMEOUT_MS,
		(long long) elapsed_ms);
}

static void wait_any_timeout() {
	Futex a(0);
	Futex b(0);
	Futex::Any<2> any{{Futex::Pair(a, 0), Futex::Pair(b, 0)}};

	const i64 start = now_ms();
	const u64 index = Futex::wait_any(any, time::duration::ms(TIMEOUT_MS));
	const i64 elapsed_ms = now_ms() - start;

	test_check(elapsed_ms >= MIN_MS, "wait_any(any, %lldms) returned index %llu after %lld ms",
		(long long) TIMEOUT_MS, (unsigned long long) index, (long long) elapsed_ms);
}

// wait_any on {idle, woken}, while another thread wakes `woken` after 50 ms.

static Futex woken(0);

static void *wake_later(void *) {
	usleep(50000);
	woken.word.store(1, atomic::Order::RELEASE);
	woken.wake_all();
	return nullptr;
}

static void wait_for_wake(time::duration timeout, u64 &index, i64 &elapsed_ms) {
	Futex idle(0);
	Futex::Any<2> any{{Futex::Pair(idle, 0), Futex::Pair(woken, 0)}};

	pthread_t t;
	test_require(pthread_create(&t, nullptr, wake_later, nullptr) == 0, "pthread_create");

	const i64 start = now_ms();
	index = Futex::wait_any(any, timeout);
	elapsed_ms = now_ms() - start;

	pthread_join(t, nullptr);
}

static void wait_any_wake() {
	u64 index;
	i64 elapsed_ms;
	wait_for_wake(time::duration::ms(1000), index, elapsed_ms);

	test_check(index == 1, "wait_any returned index %llu, expected the woken futex (1)", (unsigned long long) index);
	test_check(elapsed_ms >= 40 && elapsed_ms < 900, "wait_any(1000ms) with a wake after 50 ms returned after %lld ms",
		(long long) elapsed_ms);
}

// A timeout too large to add to the current time saturates rather than
// overflowing into a deadline in the past.
static void wait_any_maximal_timeout() {
	u64 index;
	i64 elapsed_ms;
	wait_for_wake(time::duration::ns(I64_MAX), index, elapsed_ms);

	test_check(index == 1, "wait_any returned index %llu, expected the woken futex (1)", (unsigned long long) index);
	test_check(elapsed_ms >= 40, "wait_any(I64_MAX ns) returned after %lld ms, before the wake",
		(long long) elapsed_ms);
}

static constexpr test::Case cases[] = {
	{ "wait_timeout",             wait_timeout,             5 },
	{ "wait_any_timeout",         wait_any_timeout,         5 },
	{ "wait_any_wake",            wait_any_wake,            5 },
	{ "wait_any_maximal_timeout", wait_any_maximal_timeout, 5 },
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
