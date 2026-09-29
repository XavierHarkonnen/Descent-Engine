#include <descent/thread/thread.hpp>

extern "C" {
#include <pthread.h>
}

#include "descent/thread/futex.hpp"
#include "descent/thread/mutex.hpp"
#include <descent/memory.hpp>

namespace descent::thread {

constexpr u32 MAX_THREADS = 32;
constexpr u32 STACK_SIZE = 1024 * 16;

static_assert(MAX_THREADS == 32, "MAX_THREADS must be 32");


struct ThreadData {
	alignas(memory::CACHE_LINE_SIZE)

	pthread_t pthread;
	Routine routine;
	void *argument;
	Futex signal;
	Futex finished;
	atomic::Flag owned;
	u8 _pad[31];
};

static_assert(sizeof(ThreadData) == memory::CACHE_LINE_SIZE);
static_assert(alignof(ThreadData) == memory::CACHE_LINE_SIZE);


static Atomic<u32> free(~u32(0));
ThreadData threads[MAX_THREADS];

static thread_local u32 self_index = MAX_THREADS;
static thread_local bool is_main_thread = false;


__attribute__((constructor))
static void init() {
	is_main_thread = true;
}

static u32 acquire() {
	if (!is_main_thread)
		return MAX_THREADS;

	u32 available = free.load(atomic::Order::RELAXED);
	if (!available)
		return MAX_THREADS;

	const u32 index = static_cast<u32>(bit::ctz(available));
	const u64 mask = u64(1) << index;

	free.store(available & ~mask, atomic::Order::RELAXED);

	return index;
}

static void release(u32 index) {
	free.fetch_or(u32(1) << index, atomic::Order::RELEASE);
}

static void *wrapper(void *argument) {
	ThreadData &thread = *static_cast<ThreadData *>(argument);
	self_index = u32(&thread - threads);

	thread.routine(thread.argument);

	thread.finished.word.store(1, atomic::Order::RELEASE);
	thread.finished.wake_one();

	return nullptr;
}

static u32 create_thread(Routine routine, void *argument, u64 stack_size) {
	if (!routine)
		return MAX_THREADS;

	u32 index = acquire();
	if (index == MAX_THREADS)
		return MAX_THREADS;

	threads[index].routine = routine;
	threads[index].argument = argument;
	threads[index].signal.word.store(0, atomic::Order::RELAXED);
	threads[index].finished.word.store(0, atomic::Order::RELAXED);
	threads[index].owned.set(atomic::Order::RELAXED);

	pthread_attr_t attributes;
	if (pthread_attr_init(&attributes)) {
		release(index);
		return MAX_THREADS;
	}

	if (pthread_attr_setstacksize(&attributes, stack_size)) {
		release(index);
		pthread_attr_destroy(&attributes);
		return MAX_THREADS;
	}

	if (pthread_create(&threads[index].pthread, &attributes, wrapper, &threads[index])) {
		release(index);
		pthread_attr_destroy(&attributes);
		return MAX_THREADS;
	}

	pthread_attr_destroy(&attributes);

	return index;
}

Thread::Thread(Routine routine, void *argument) : Thread() {
	u32 index = create_thread(routine, argument, STACK_SIZE);
	if (index == MAX_THREADS)
		return;

	_.store(index, atomic::Order::RELAXED);
}

Thread::Thread(Routine routine, void *argument, u64 stack_size) : Thread() {
	u32 index = create_thread(routine, argument, stack_size);
	if (index == MAX_THREADS)
		return;

	_.store(index, atomic::Order::RELAXED);
}

Thread::~Thread() {
	mutex::Scope scope(lock);

	u32 index = _.load(atomic::Order::RELAXED);
	if (index != INVALID)
		threads[index].owned.clear(atomic::Order::RELAXED);
}

bool Thread::join() {
	lock.lock();

	u32 index = _.load(atomic::Order::RELAXED);
	if (index == self_index)
		index = INVALID;
	else
		_.store(INVALID, atomic::Order::RELAXED);

	lock.unlock();

	if (index == INVALID)
		return false;

	ThreadData &thread = threads[index];

	if (pthread_join(thread.pthread, nullptr))
		while (thread.finished.word.load(atomic::Order::ACQUIRE) == 0)
			thread.finished.wait(0);
	
	release(index);
	return true;
}

bool Thread::send(u32 signals) {
	mutex::Scope scope(lock);

	u32 index = _.load(atomic::Order::RELAXED);
	if (index == INVALID)
		return false;

	threads[index].signal.word.fetch_or(signals, atomic::Order::RELEASE);

	return true;
}

bool Thread::wake(u32 signals) {
	mutex::Scope scope(lock);

	u32 index = _.load(atomic::Order::RELAXED);
	if (index == INVALID)
		return false;

	threads[index].signal.word.fetch_or(signals, atomic::Order::RELEASE);
	threads[index].signal.wake_one();

	return true;
}

u32 Thread::poll() {
	if (self_index == MAX_THREADS)
		return 0;

	return threads[self_index].signal.word.exchange(0, atomic::Order::ACQUIRE);
}

u32 Thread::wait() {
	if (self_index == MAX_THREADS)
		return 0;

	for (;;) {
		u32 result = poll();
		if (result != 0)
			return result;

		threads[self_index].signal.wait(0);
	}
}

u32 Thread::maximum() {
	return MAX_THREADS;
}

u32 Thread::collect() {
	const u64 available = free.load(atomic::Order::RELAXED);

	if (is_main_thread) {
		for (u64 i = 0; i < MAX_THREADS; ++i) {
			ThreadData &thread = threads[i];

			if (available & (u32(1) << i))
				continue;

			if (thread.owned.load(atomic::Order::RELAXED))
				continue;

			Thread temp;
			temp._.store(static_cast<u32>(&thread - threads), atomic::Order::RELAXED);
			temp.join();
		}
	}
	
	return static_cast<u32>(bit::popcnt(~free.load(atomic::Order::RELAXED)));
}

}