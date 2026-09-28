#include <descent/thread/task.hpp>

extern "C" {
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
}

#include <descent/memory.hpp>
#include <descent/system.hpp>
#include <descent/thread/futex.hpp>
#include <descent/thread/mutex.hpp>
#include <descent/type.hpp>

#include "task/worker.hpp"
#include "task/pool.hpp"
#include "task/state.hpp"

namespace descent::thread::task {

namespace state {
	static Pool _pool(16);
	static thread_local Worker *_self = nullptr;

	Worker *self() { return _self; }
	void self(Worker *self) { _self = self; }
};

bool Handle::spawn(Job job) {
	return _->spawn(job);
}

bool Task::spawn(const Job &job) {
	if (!job.routine)
		return false;

	Worker *self = state::self();
	if (!self)
		return false;
	
	Task *task = self->allocate();
	if (!task)
		return false;

	task->create(job, *this);
	self->submit(task);

	return true;
}

bool Task::spawn(const Job &job, u64 size) {
	if (!job.routine)
		return false;

	sys_assert(size <= INLINE_DATA_SIZE, "Attempted to set task with oversized data");
	
	Worker *self = state::self();
	if (!self)
		return false;
	
	Task *task = self->allocate();
	if (!task)
		return false;

	memcpy(task->_data, job.context, size);
	task->create({job.routine, task->_data}, *this);
	self->submit(task);

	return true;
}

bool frame::begin(const Job *jobs, u64 count) {
	return state::_pool.begin(jobs, count);
}

u32 frame::poll() {
	return state::_pool.poll();
}

bool frame::end() {
	return state::_pool.end();
}

u64 diagnostic::workers() {
	return state::_pool.count();
}

}
