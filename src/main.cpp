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

#include <cstdio>

#include <descent/thread/atomic.hpp>
#include <descent/thread/task.hpp>
#include <descent/thread/thread.hpp>
#include <descent/time.hpp>

#include <descent/random.hpp>
#include <descent/memory/stack.hpp>

#include <pthread.h>

using namespace descent;

constexpr u64 CHILDREN = 18;
constexpr u64 FRAMES = 3;

struct context {
	thread::Atomic<u64> executed;
	thread::Atomic<u64> failed;
	thread::Atomic<u64> orphaned;
};

void work4(thread::task::Handle handle, void *arg) {
	(void) handle;
	context &context = *static_cast<struct context *>(arg);
	context.executed.fetch_add(1, thread::atomic::Order::RELAXED);
}

void work3(thread::task::Handle handle, void *arg) {
	context &context = *static_cast<struct context *>(arg);
	context.executed.fetch_add(1, thread::atomic::Order::RELAXED);

	for (u64 i = 0; i < CHILDREN; ++i)
		if (!handle.spawn({work4, arg})) {
			context.failed.fetch_add(1, thread::atomic::Order::RELAXED);
		}
}

void work2(thread::task::Handle handle, void *arg) {
	context &context = *static_cast<struct context *>(arg);
	context.executed.fetch_add(1, thread::atomic::Order::RELAXED);

	for (u64 i = 0; i < CHILDREN; ++i)
		if (!handle.spawn({work3, arg})) {
			context.failed.fetch_add(1, thread::atomic::Order::RELAXED);
			context.orphaned.fetch_add(CHILDREN, thread::atomic::Order::RELAXED);
		}
}

void work1(thread::task::Handle handle, void *arg) {
	context &context = *static_cast<struct context *>(arg);
	context.executed.fetch_add(1, thread::atomic::Order::RELAXED);

	for (u64 i = 0; i < CHILDREN; ++i)
		if (!handle.spawn({work2, arg})) {
			context.failed.fetch_add(1, thread::atomic::Order::RELAXED);
			context.orphaned.fetch_add(CHILDREN + CHILDREN * CHILDREN, thread::atomic::Order::RELAXED);
		}
}

void dedicated(void *) {
	time::sleep(1000000000);
	puts("Hello, world!");
	pthread_detach(pthread_self());
}

int main() {
	u64 workers = thread::task::get();
	printf("Worker count: %lu\n", workers);

	struct context context;

	thread::task::Job jobs[] = {
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context },
		{ work1, &context }
	};

	for (u64 i = 0; i < FRAMES; ++i) {
		printf("Starting frame\n");
		thread::task::begin(jobs);

		u32 unfinished;
		while((unfinished = thread::task::poll()) != 0);

		thread::task::end();
	}

	u64 tier_1 = sizeof(jobs) / sizeof(jobs[0]);
	u64 tier_2 = tier_1 * CHILDREN;
	u64 tier_3 = tier_2 * CHILDREN;
	u64 tier_4 = tier_3 * CHILDREN;
	u64 total = tier_1 + tier_2 + tier_3 + tier_4;

	u64 expected = FRAMES * total;
	u64 local  = FRAMES * (tier_1 + 1023) * workers;
	u64 maximum  = FRAMES * (tier_1 + 1023 + 6 * 1024) * workers;
	u64 executed = context.executed.load(thread::atomic::Order::RELAXED);
	u64 failed   = context.failed.load(thread::atomic::Order::RELAXED);
	u64 orphaned = context.orphaned.load(thread::atomic::Order::RELAXED);
	u64 lost = expected - executed - failed - orphaned;

	printf("Expected : %lu\n", expected);
	printf("Local    : %lu\n", local);
	printf("Maximum  : %lu\n", maximum);
	printf("Executed : %lu\n", executed);
	printf("Failed   : %lu\n", failed);
	printf("Orphaned : %lu\n", orphaned);
	printf("Lost     : %lu\n", lost);
	printf("Deficit  : %f%%\n", expected > maximum ? f32(expected - maximum) / f32(expected) * 100.f : 0.f);
	printf("Failure  : %f%%\n", f32(failed + orphaned + lost) / f32(expected) * 100.f);

	thread::Thread t(dedicated, nullptr);
	for (u64 i = 0; i < thread::Thread::maximum() - 1; ++i) {
		thread::Thread(dedicated, nullptr);
	}

	u32 remaining = thread::Thread::collect();
	printf("%u threads remain after collection\n", remaining);
	
	t.join();
	puts("Joined thread t");

	{
		auto data = memory::stack::Allocator::create<u8>(12);
		data[0] = 1;
		{

			
		}
	}
	
	return 0;
}
