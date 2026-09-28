#ifndef DESCENT_LIB_THREAD_TASK_POOL_HPP
#define DESCENT_LIB_THREAD_TASK_POOL_HPP

extern "C" {
#include <sys/sysinfo.h>
}

#include <descent/thread/mutex.hpp>
#include <descent/type.hpp>

#include "worker.hpp"
#include "state.hpp"

namespace descent::thread::task {

class Pool {
private:
	static constexpr u64 MAX = 16;

	Worker _workers[MAX];
	Root _root;
	u64 _count;
	Mutex _mutex;
	Futex _finished;
	bool _active;

	void init(u64 desired) {
		for (u64 index = 0; index < desired; ++index) {
			if (!_workers[index].init())
				break;

			++_count;
		}
	}

	void configure() {
		for (u64 index = 0; index < _count; ++index)
			_workers[index].configure(&_finished, _workers, _count, index);
	}

	void terminate() {
		for (u64 index = 0; index < _count; ++index) {
			_workers[index].terminate();
		}
	}

	void join() {
		for (u64 index = 0; index < _count; ++index) {
			_workers[index].join();
		}
	}

	void start() {
		for (u64 index = 0; index < _count; ++index)
			_workers[index].start();
	}

	void inject(const Job *jobs, u64 count) {
		for (u64 i = 0; i < count; ++i) {
			_workers[i % _count].inject(jobs[i], _root);
		}
	}

	void stop() {
		for (u64 i = 0; i < _count; ++i) {
			_workers[i].stop();
		}
		
		for (;;) {
			const u32 finished = _finished.word.load(atomic::Order::ACQUIRE);

			if (finished == _count)
				break;

			_finished.wait(finished);
		}
	}

	void clear() {
		for (u64 index = 0; index < _count; ++index) {
			_workers[index].clear();
		}

		for (;;) {
			const u32 finished = _finished.word.load(atomic::Order::ACQUIRE);

			if (finished == _count)
				break;

			_finished.wait(finished);
		}
	}

public:

	Pool(u64 desired) : _count(0), _finished(0), _active(false) {
		init(desired < MAX ? desired : MAX);
		configure();
	}

	~Pool() {
		terminate();
		join();
	}

	bool begin(const Job *jobs, u64 count) {
		mutex::Scope scope(_mutex);

		if (_active)
			return false;

		_active = true;

		_root.init();

		_finished.word.store(0, atomic::Order::RELEASE);

		inject(jobs, count);

		_root.release();

		start();

		return true;
	}

	u32 poll() {
		return _root.poll();
	}

	bool end() {
		mutex::Scope scope(_mutex);

		if (!_active)
			return true;

		if (poll() != 0)
			return false;

		stop();

		clear();

		_active = false;

		return true;
	}

	u64 count() const {
		return _count;
	}

	// Exposing internals - bad, fix

	Root &root() {
		return _root;
	}
};

}

#endif