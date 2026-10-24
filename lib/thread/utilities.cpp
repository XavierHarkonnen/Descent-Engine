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

#include <descent/utils.hpp>

#include <descent/system.hpp>
#include <descent/thread/atomic.hpp>
#include <descent/type/core.hpp>

namespace descent::thread {

static constexpr u64 ID_MAIN = 0;
static constexpr u64 ID_INVALID = ID_MAIN - 1;
static constexpr u64 ID_FIRST = ID_MAIN + 1;

static thread::Atomic<u64> id_next(ID_FIRST);
static thread_local u64 id_thread = ID_INVALID;

u64 id() {
	if (id_thread == ID_INVALID) {
		id_thread = id_next.fetch_add(1, thread::atomic::Order::RELAXED);
		sys_assert(id_thread != ID_INVALID, "Encountered thread ID wraparound");
	}

	return id_thread;
}

[[gnu::constructor]]
static void init() {
	id_thread = ID_MAIN;
}

}
