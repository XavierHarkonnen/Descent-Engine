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

#ifndef DESCENT_LIB_THREAD_TASK_TASK_HPP
#define DESCENT_LIB_THREAD_TASK_TASK_HPP

#include <descent/memory/platform.hpp>
#include <descent/system.hpp>
#include <descent/thread/atomic.hpp>
#include <descent/thread/futex.hpp>
#include <descent/thread/task.hpp>
#include <descent/type/core.hpp>

namespace descent::thread::task {

class Node {
	friend class Task;

	Node(Node &&other) = delete;
	Node(const Node &other) = delete;
	Node &operator=(Node &&other) = delete;
	Node &operator=(const Node &other) = delete;

protected:
	alignas(memory::CACHE_LINE_SIZE)

	Routine _routine;
	void *_context;
	Node *_parent;
	Atomic<u32> _unfinished;
	u8 _data[INLINE_DATA_SIZE];

	Node(Routine routine, void *context, Node *parent, u32 unfinished)
	: _routine(routine), _context(context), _parent(parent), _unfinished(unfinished) {}

	u32 increment() {
		return _unfinished.add_fetch(1, atomic::Order::RELEASE);
	}

	u32 decrement() {
		return _unfinished.sub_fetch(1, atomic::Order::RELEASE);
	}

	void finish() {
		const u32 unfinished = decrement();

		if ((unfinished == 0 && _parent)) {
			_parent->finish();
		}
	}

public:
	u32 poll() {
		return _unfinished.load(atomic::Order::ACQUIRE);
	}
};

static_assert(sizeof(Node) == memory::CACHE_LINE_SIZE);
static_assert(alignof(Node) == memory::CACHE_LINE_SIZE);

class Root : public Node {
public:
	Root() : Node(nullptr, nullptr, nullptr, 0) {}

	void init() {
		sys_assert(poll() == 0, "Attempted to initialize existing root");

		_routine = nullptr;
		_context = nullptr;
		_parent = nullptr;
		_unfinished.store(1, atomic::Order::RELEASE);
	}

	void release() {
		[[maybe_unused]] u32 unfinished = decrement();

		sys_assert(unfinished + 1 != 0, "Attempted to release an uninitialized root");
	}
};

class Task : public Node {
private:
	void create(const Job &job, Node &parent) {
		sys_assert(poll() == 0, "Attempted to initialize existing task");
		sys_assert(job.routine, "Attempted to initialize task with null routine");

		_routine = job.routine;
		_context = job.context;
		_parent = &parent;
		_unfinished.store(1, atomic::Order::RELEASE);
		parent.increment();
	}

public:
	Task() : Node(nullptr, nullptr, nullptr, 0) {}

	void init(const Job &job, Root &root) {
		create(job, root);
	}

	void operator ()() {
		sys_assert(_routine, "Attempted to execute uninitialized task");

		_routine(Handle(this), _context);

		finish();
	}

	bool spawn(const Job &job);

	bool spawn(const Job &job, u64 size);
};

}

#endif