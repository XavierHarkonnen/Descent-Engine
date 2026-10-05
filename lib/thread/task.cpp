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

static Pool _pool;
static thread_local Worker *_self = nullptr;

Worker *self() {
	return _self;
}

void self(Worker *self) {
	_self = self;
}

bool Handle::spawn(Job job) {
	return _->spawn(job);
}

bool Task::spawn(const Job &job) {
	if (!job.routine)
		return false;

	Worker *host = self();
	if (!host)
		return false;
	
	Task *task = host->allocate();
	if (!task)
		return false;

	task->create(job, *this);
	host->submit(task);

	return true;
}

bool Task::spawn(const Job &job, u64 size) {
	if (!job.routine)
		return false;

	sys_assert(size <= INLINE_DATA_SIZE, "Attempted to set task with oversized data");
	
	Worker *host = self();
	if (host)
		return false;
	
	Task *task = host->allocate();
	if (!task)
		return false;

	memcpy(task->_data, job.context, size);
	task->create({job.routine, task->_data}, *this);
	host->submit(task);

	return true;
}

u64 set(u64 count) {
	return _pool.set(count);
}

u64 get() {
	return _pool.get();
}

bool begin(const Job *jobs, u64 count) {
	if (count == 0 || count > MAX_START_JOBS)
		return false;

	for (u64 i = 0; i < count; ++i)
		if (!jobs[i].routine)
			return false;

	return _pool.begin(jobs, count);
}

u32 poll() {
	return _pool.poll();
}

bool end() {
	return _pool.end();
}

}
