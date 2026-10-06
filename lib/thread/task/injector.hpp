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

#ifndef DESCENT_LIB_THREAD_TASK_INJECTOR_HPP
#define DESCENT_LIB_THREAD_TASK_INJECTOR_HPP

#include <descent/system.hpp>
#include <descent/thread/task.hpp>
#include <descent/type.hpp>

#include "node.hpp"

namespace descent::thread::task {

class Injector {
private:
	Task _tasks[MAX_START_JOBS];
	u64 _cursor;
public:
	Injector() : _cursor(0) {}

  void push(const Job &job, Root &root) {
		sys_assert(_cursor < MAX_START_JOBS, "Attempted to inject more than MAX_START_JOBS");

		_tasks[_cursor++].init(job, root);
	}

	void drain() {
		while (_cursor != 0) {
			--_cursor;
			_tasks[_cursor]();
		}
	}
};

}

#endif