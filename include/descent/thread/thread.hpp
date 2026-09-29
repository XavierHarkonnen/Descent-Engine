#ifndef DESCENT_THREAD_THREAD_HPP
#define DESCENT_THREAD_THREAD_HPP

#include <descent/thread/atomic.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/type.hpp>

namespace descent::thread {

using Routine = void (*)(void *);

class Thread {
	Thread(const Thread &) = delete;
	Thread &operator=(const Thread &) = delete;

private:
	static constexpr u32 INVALID = ~u32(0);

	Atomic<u32> _;
	Mutex lock;

public:
	Thread() : _(INVALID) {}

	Thread(Routine routine, void *argument);

	Thread(Routine routine, void *argument, u64 stack_size);

	Thread(Thread &&other) {
		mutex::Scope scope(other.lock);

		u32 index = other._.load(atomic::Order::RELAXED);
		_.store(index, atomic::Order::RELAXED);
		other._.store(INVALID, atomic::Order::RELAXED);
	};

	~Thread();
	
	Thread &operator=(Thread &&other) {
		if (this != &other) {
			Thread *t0 = this > &other ? this : &other;
			Thread *t1 = this > &other ? &other : this;

			mutex::Scope s0(t0->lock);
			mutex::Scope s1(t1->lock);

			u32 index = other._.load(atomic::Order::RELAXED);
			_.store(index, atomic::Order::RELAXED);
			other._.store(INVALID, atomic::Order::RELAXED);
		}

		return *this;
	}

	explicit operator bool() const {
		return _.load(atomic::Order::RELAXED) != INVALID;
	}

	bool join();

	// Send a signal bitset to this thread
	bool send(u32 signals);

	// Send a signal bitset to this thread and wake it
	bool wake(u32 signals);
	
	// Read outstanding signal bitset
	static u32 poll();

	// Wait for and read outstanding signal bitset
	static u32 wait();

	// Maximum number of threads
	static u32 maximum();

	static u32 collect();
};
}

#endif