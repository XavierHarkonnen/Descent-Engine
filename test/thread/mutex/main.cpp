// thread::Mutex: mutual exclusion, timed and non-blocking locking, and wakeup
// of every waiter.

extern "C" {
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
}

#include <harness.hpp>

#include <descent/thread/atomic.hpp>
#include <descent/thread/futex.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/time.hpp>

using namespace descent;
using namespace descent::thread;

static i64 now_ms() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return i64(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

// Mutual exclusion under heavy contention: a non-atomic counter must reach
// the exact total.

static constexpr u32 THREADS = 8;
static constexpr u64 ITERATIONS = 20000;

static Mutex counter_mutex;
static u64 counter = 0;

static void *increment(void *) {
	for (u64 i = 0; i < ITERATIONS; ++i) {
		mutex::Scope scope(counter_mutex);
		u64 value = counter;
		// Widen the critical section so overlap would be visible.
		__asm__ __volatile__("" ::: "memory");
		counter = value + 1;
	}
	return nullptr;
}

static void exclusion() {
	pthread_t threads[THREADS];
	for (pthread_t &t : threads)
		test_require(pthread_create(&t, nullptr, increment, nullptr) == 0, "pthread_create");
	for (pthread_t &t : threads)
		pthread_join(t, nullptr);

	test_check(counter == THREADS * ITERATIONS, "counter = %llu, expected %llu", (unsigned long long) counter,
		(unsigned long long) (THREADS * ITERATIONS));
}

// lock(timeout)

static Mutex timed_mutex;
static bool timed_result = true;
static i64 timed_elapsed_ms = 0;

static void *timed_locker(void *) {
	const i64 start = now_ms();
	timed_result = timed_mutex.lock(time::duration::ms(100));
	timed_elapsed_ms = now_ms() - start;
	if (timed_result)
		timed_mutex.unlock();
	return nullptr;
}

static void timed_lock_expires() {
	timed_mutex.lock();

	pthread_t t;
	test_require(pthread_create(&t, nullptr, timed_locker, nullptr) == 0, "pthread_create");
	pthread_join(t, nullptr);

	timed_mutex.unlock();

	test_check(!timed_result, "lock(100ms) acquired a held mutex");
	test_check(timed_elapsed_ms >= 75, "lock(100ms) gave up after %lld ms", (long long) timed_elapsed_ms);
}

static void *release_later(void *) {
	usleep(50000);
	timed_mutex.unlock();
	return nullptr;
}

static void timed_lock_acquires() {
	timed_mutex.lock();

	pthread_t t;
	test_require(pthread_create(&t, nullptr, release_later, nullptr) == 0, "pthread_create");

	const i64 start = now_ms();
	const bool acquired = timed_mutex.lock(time::duration::ms(1000));
	const i64 elapsed_ms = now_ms() - start;

	pthread_join(t, nullptr);
	if (acquired)
		timed_mutex.unlock();

	test_check(acquired, "lock(1000ms) timed out although the holder released after 50 ms");
	test_check(elapsed_ms < 900, "lock(1000ms) acquired after %lld ms", (long long) elapsed_ms);
}

// trylock()

static Mutex try_mutex;
static bool try_while_held = true;

static void *try_other(void *) {
	try_while_held = try_mutex.trylock();
	return nullptr;
}

static void trylock_semantics() {
	try_mutex.lock();
	pthread_t t;
	test_require(pthread_create(&t, nullptr, try_other, nullptr) == 0, "pthread_create");
	pthread_join(t, nullptr);
	try_mutex.unlock();

	const bool when_free = try_mutex.trylock();
	if (when_free)
		try_mutex.unlock();

	test_check(!try_while_held, "trylock succeeded while another thread held the mutex");
	test_check(when_free, "trylock failed on a free mutex");
}

// Every waiter wakes. With the mutex held, two threads block in lock() and
// sleep with the word CONTENDED. One unlock must eventually let both acquire.
// A mutex that re-acquires as LOCKED rather than CONTENDED after sleeping
// loses the second wakeup (fixed in 0fb2dce). Repeated, so one lucky
// schedule cannot hide the defect.

// Mutex keeps its Futex private, and it is Mutex's only member, so the word
// is read through the object's address.
static u32 word_of(Mutex &m) {
	return reinterpret_cast<Futex *>(&m)->word.load(atomic::Order::ACQUIRE);
}

struct Waiters {
	Mutex mutex;
	Atomic<u32> tids[2];
	Atomic<u32> done{0};
};

struct Waiter {
	Waiters *shared;
	u32 slot;
};

static void *waiter(void *p) {
	Waiter &arg = *static_cast<Waiter *>(p);
	arg.shared->tids[arg.slot].store(u32(syscall(SYS_gettid)), atomic::Order::RELEASE);

	arg.shared->mutex.lock();
	arg.shared->mutex.unlock();

	arg.shared->done.fetch_add(1, atomic::Order::RELEASE);
	return nullptr;
}

// True once the thread is in interruptible sleep ('S').
static bool sleeping(u32 tid) {
	char path[64];
	snprintf(path, sizeof(path), "/proc/self/task/%u/stat", tid);

	FILE *f = fopen(path, "r");
	if (!f)
		return false;

	char buffer[512] = {};
	const size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
	fclose(f);
	buffer[n] = '\0';

	// Format: "tid (comm) S ..."; comm may contain spaces, so find the last ')'.
	const char *close = strrchr(buffer, ')');
	return close && close[1] == ' ' && close[2] == 'S';
}

static void every_waiter_wakes() {
	constexpr u32 RUNS = 5;

	for (u32 run = 0; run < RUNS; ++run) {
		Waiters shared;
		Waiter args[2] = {{&shared, 0}, {&shared, 1}};

		shared.mutex.lock();

		pthread_t threads[2];
		for (u32 i = 0; i < 2; ++i)
			test_require(pthread_create(&threads[i], nullptr, waiter, &args[i]) == 0, "pthread_create");

		// Wait until both are asleep and the word is CONTENDED.
		i64 deadline = now_ms() + 2000;
		for (;;) {
			const u32 t0 = shared.tids[0].load(atomic::Order::ACQUIRE);
			const u32 t1 = shared.tids[1].load(atomic::Order::ACQUIRE);
			if (t0 && t1 && sleeping(t0) && sleeping(t1) && word_of(shared.mutex) == 2)
				break;

			test_require(now_ms() < deadline, "run %u: setup: both waiters never slept on a contended mutex", run);
			usleep(1000);
		}

		// Let both settle fully inside futex_wait.
		usleep(100000);

		shared.mutex.unlock();

		deadline = now_ms() + 2000;
		while (shared.done.load(atomic::Order::ACQUIRE) != 2 && now_ms() < deadline)
			usleep(1000);

		const u32 done = shared.done.load(atomic::Order::ACQUIRE);
		test_require(done == 2, "run %u: %u of 2 waiters acquired the mutex within 2 s; word = %u", run, done,
			word_of(shared.mutex));

		for (pthread_t &t : threads)
			pthread_join(t, nullptr);
	}
}

static constexpr test::Case cases[] = {
	{ "exclusion",           exclusion,           20 },
	{ "timed_lock_expires",  timed_lock_expires,  5 },
	{ "timed_lock_acquires", timed_lock_acquires, 5 },
	{ "trylock_semantics",   trylock_semantics,   5 },
	{ "every_waiter_wakes",  every_waiter_wakes,  20 },
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
