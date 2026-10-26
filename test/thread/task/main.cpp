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

// thread::task: frames of root jobs and spawned children across every worker
// count, pool resizing, and task::wait().

extern "C" {
#include <time.h>
#include <unistd.h>
}

#include <harness.hpp>

#include <descent/thread/atomic.hpp>
#include <descent/thread/task.hpp>

using namespace descent;
using namespace descent::thread;

static i64 now_ms() {
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return i64(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

static i64 thread_cpu_ms() {
	timespec ts;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	return i64(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

static void noop(task::Handle, void *) {}

// Waits up to timeout_ms for the frame to drain. Returns whether it did.
static bool drain(i64 timeout_ms) {
	const i64 deadline = now_ms() + timeout_ms;
	while (task::poll() != 0 && now_ms() < deadline)
		usleep(100);
	return task::poll() == 0;
}

// Every worker count: two consecutive frames (pool reuse), each of 64 root
// jobs that each spawn 8 children. Every job runs exactly once, each frame
// drains, and end() succeeds.

static constexpr u64 ROOTS    = task::MAX_START_JOBS;
static constexpr u64 CHILDREN = 8;

static Atomic<u64> roots_ran{ 0 };
static Atomic<u64> children_ran{ 0 };

static void child(task::Handle, void *) {
	children_ran.fetch_add(1, atomic::Order::RELAXED);
}

static void root(task::Handle self, void *) {
	roots_ran.fetch_add(1, atomic::Order::RELAXED);
	for (u64 i = 0; i < CHILDREN; ++i)
		self.spawn(task::Job{ child, nullptr });
}

template<u64 WORKERS>
static void frames() {
	test_require(task::set(WORKERS) == WORKERS, "set(%llu)", (unsigned long long) WORKERS);

	task::Job jobs[ROOTS];
	for (task::Job &job : jobs)
		job = task::Job{ root, nullptr };

	for (u32 frame = 0; frame < 2; ++frame) {
		roots_ran.store(0, atomic::Order::RELAXED);
		children_ran.store(0, atomic::Order::RELAXED);

		test_require(task::begin(jobs, ROOTS), "frame %u: begin", frame);
		test_require(drain(3000), "frame %u: %u task(s) still unfinished after 3 s", frame, task::poll());
		test_require(task::end(), "frame %u: end", frame);

		test_check(
			roots_ran.load(atomic::Order::RELAXED) == ROOTS, "frame %u: %llu of %llu roots ran", frame,
			(unsigned long long) roots_ran.load(atomic::Order::RELAXED), (unsigned long long) ROOTS
		);
		test_check(
			children_ran.load(atomic::Order::RELAXED) == ROOTS * CHILDREN, "frame %u: %llu of %llu children ran", frame,
			(unsigned long long) children_ran.load(atomic::Order::RELAXED), (unsigned long long) (ROOTS * CHILDREN)
		);
	}

	task::set(0);
}

// A frame with two workers runs to completion. Worker::victim() once asked
// for an empty random range here (fixed in 7259d86).
static void two_workers() {
	static Atomic<u32> ran{ 0 };
	auto job = [](task::Handle, void *) { ran.fetch_add(1, atomic::Order::RELEASE); };

	test_require(task::set(2) == 2, "set(2)");
	task::Job jobs[] = { { job, nullptr }, { job, nullptr } };
	test_require(task::begin(jobs, 2), "begin");
	test_require(drain(5000), "frame did not drain");
	task::end();
	task::set(0);

	test_check(ran.load(atomic::Order::ACQUIRE) == 2, "%u of 2 jobs ran", ran.load(atomic::Order::ACQUIRE));
}

// Frames with fewer root jobs than workers. If the frame completes before a
// worker has run, end() must still see every worker finish (fixed in 503d990).

static void one_root_frames() {
	constexpr u32 FRAMES = 5000;

	test_require(task::set(4) == 4, "set(4)");
	task::Job jobs[] = { { noop, nullptr } };

	for (u32 frame = 0; frame < FRAMES; ++frame) {
		test_require(task::begin(jobs), "frame %u: begin", frame);
		while (task::poll() != 0) {
		}
		test_require(task::end(), "frame %u: end", frame);
	}

	task::set(0);
}

// The same, growing the pool one worker at a time, so newly started workers
// are exercised as well as long-lived ones.
static void one_root_frames_growing_pool() {
	constexpr u32 FRAMES = 300;
	task::Job jobs[]     = { { noop, nullptr } };

	for (u64 workers = 1; workers <= task::MAX_WORKERS; ++workers) {
		test_require(task::set(workers) == workers, "set(%llu)", (unsigned long long) workers);

		for (u32 frame = 0; frame < FRAMES; ++frame) {
			test_require(task::begin(jobs), "%llu workers, frame %u: begin", (unsigned long long) workers, frame);
			while (task::poll() != 0) {
			}
			test_require(task::end(), "%llu workers, frame %u: end", (unsigned long long) workers, frame);
		}
	}

	task::set(0);
}

// Repeated shrink and regrowth of the pool. A restarted worker must not exit
// on the terminate state its previous thread left behind (fixed in d4a8dac).

static void resize(u64 roots_per_frame, u32 rounds) {
	static constexpr u64 SEQUENCE[] = { 16, 1, 16, 2, 8, 1, 4, 16, 3, 12, 1, 16 };
	constexpr u32 FRAMES            = 20;

	task::Job jobs[task::MAX_WORKERS];
	for (task::Job &job : jobs)
		job = task::Job{ noop, nullptr };

	for (u32 round = 0; round < rounds; ++round) {
		for (u64 step = 0; step < sizeof(SEQUENCE) / sizeof(SEQUENCE[0]); ++step) {
			const u64 workers = SEQUENCE[step];
			test_require(task::set(workers) == workers, "set(%llu)", (unsigned long long) workers);

			for (u32 frame = 0; frame < FRAMES; ++frame) {
				test_require(task::begin(jobs, roots_per_frame ? roots_per_frame : workers), "begin");

				const i64 deadline = now_ms() + 2000;
				while (task::poll() != 0 && now_ms() < deadline) {
				}

				test_require(
					task::poll() == 0, "round %u, step %llu (set %llu after %llu), frame %u: %u task(s) never ran",
					round, (unsigned long long) step, (unsigned long long) workers,
					(unsigned long long) (step ? SEQUENCE[step - 1] : 0), frame, task::poll()
				);
				test_require(task::end(), "end");
			}
		}
	}

	task::set(0);
}

// One root job per worker, so no worker can miss a frame start. Every worker
// must run in every frame, so frames are slow when workers outnumber CPUs, and
// one pass through the sequence is enough: a worker that exits on restart is
// caught at the first regrowth.
static void resize_root_per_worker() {
	resize(0, 1);
}

// Single-job frames leave most workers idle, so restarted workers must also
// catch frames they do not run in.
static void resize_one_root() {
	resize(1, 25);
}

// task::wait() sleeps until the frame's unfinished count reaches zero.

static void wait_inactive() {
	const i64 start = now_ms();
	task::wait();
	const i64 elapsed_ms = now_ms() - start;

	test_check(elapsed_ms < 10, "wait() with no frame took %lld ms", (long long) elapsed_ms);
}

static constexpr u64 SLOW_ROOTS    = 4;
static constexpr u64 SLOW_CHILDREN = 4;
static Atomic<u64> slow_finished{ 0 };

static void slow_child(task::Handle, void *) {
	usleep(100000);
	slow_finished.fetch_add(1, atomic::Order::RELEASE);
}

static void slow_root(task::Handle self, void *) {
	usleep(100000);
	for (u64 i = 0; i < SLOW_CHILDREN; ++i)
		self.spawn(task::Job{ slow_child, nullptr });
	slow_finished.fetch_add(1, atomic::Order::RELEASE);
}

// wait() returns only after every task, children included, has finished, and
// sleeps rather than spins while it waits.
static void wait_covers_children() {
	test_require(task::set(4) == 4, "set(4)");

	task::Job jobs[SLOW_ROOTS];
	for (task::Job &job : jobs)
		job = task::Job{ slow_root, nullptr };
	test_require(task::begin(jobs, SLOW_ROOTS), "begin");

	const i64 start     = now_ms();
	const i64 cpu_start = thread_cpu_ms();
	task::wait();
	const i64 cpu_ms     = thread_cpu_ms() - cpu_start;
	const i64 elapsed_ms = now_ms() - start;

	const u64 done   = slow_finished.load(atomic::Order::ACQUIRE);
	const u32 left   = task::poll();
	const bool ended = task::end();
	task::set(0);

	constexpr u64 TOTAL = SLOW_ROOTS + SLOW_ROOTS * SLOW_CHILDREN;
	test_check(
		done == TOTAL, "%llu of %llu tasks had finished when wait() returned", (unsigned long long) done,
		(unsigned long long) TOTAL
	);
	test_check(left == 0, "poll() = %u after wait()", left);
	test_check(ended, "end() failed after wait()");
	test_check(elapsed_ms >= 180, "wait() returned after %lld ms; the tasks take about 200 ms", (long long) elapsed_ms);
	test_check(
		cpu_ms < 20, "the waiting thread used %lld ms of CPU in %lld ms", (long long) cpu_ms, (long long) elapsed_ms
	);
}

static void quick(task::Handle self, void *context) {
	if (context)
		self.spawn(task::Job{ quick, nullptr });
}

// Many short frames in a row: wait() never misses the final wake.
static void wait_many_frames() {
	constexpr u32 FRAMES = 2000;

	test_require(task::set(4) == 4, "set(4)");

	task::Job jobs[8];
	for (task::Job &job : jobs)
		job = task::Job{ quick, reinterpret_cast<void *>(1) };

	for (u32 frame = 0; frame < FRAMES; ++frame) {
		test_require(task::begin(jobs, 8), "frame %u: begin", frame);
		task::wait();
		test_require(task::poll() == 0, "frame %u: wait() returned with %u task(s) outstanding", frame, task::poll());
		test_require(task::end(), "frame %u: end", frame);
	}

	task::set(0);
}

// wait() from inside a task cannot complete, since the task itself is
// unfinished. Debug builds trap it; release builds return immediately.

static void waits_on_itself(task::Handle, void *) {
	task::wait();
}

static void wait_from_task() {
	test_require(task::set(2) == 2, "set(2)");

	// One root job per worker, so no worker can miss the frame start.
	task::Job jobs[] = { { waits_on_itself, nullptr }, { quick, nullptr } };
	test_require(task::begin(jobs), "begin");

	task::wait();
	task::end();
	task::set(0);
}

#if defined(DESCENT_DEBUG)
static constexpr const char *WAIT_FROM_TASK_TRAP = "Attempted to wait on the task frame from a task";
#else
static constexpr const char *WAIT_FROM_TASK_TRAP = nullptr;
#endif

static constexpr test::Case cases[] = {
	{ "frames_1_worker", frames<1> },
	{ "frames_2_workers", frames<2> },
	{ "frames_3_workers", frames<3> },
	{ "frames_4_workers", frames<4> },
	{ "frames_5_workers", frames<5> },
	{ "frames_6_workers", frames<6> },
	{ "frames_7_workers", frames<7> },
	{ "frames_8_workers", frames<8> },
	{ "frames_9_workers", frames<9> },
	{ "frames_10_workers", frames<10> },
	{ "frames_11_workers", frames<11> },
	{ "frames_12_workers", frames<12> },
	{ "frames_13_workers", frames<13> },
	{ "frames_14_workers", frames<14> },
	{ "frames_15_workers", frames<15> },
	{ "frames_16_workers", frames<16> },
	{ "two_workers", two_workers },
	{ "one_root_frames", one_root_frames, 20 },
	{ "one_root_frames_growing_pool", one_root_frames_growing_pool, 30 },
	{ "resize_root_per_worker", resize_root_per_worker, 60 },
	{ "resize_one_root", resize_one_root, 60 },
	{ "wait_inactive", wait_inactive },
	{ "wait_covers_children", wait_covers_children },
	{ "wait_many_frames", wait_many_frames, 30 },
	{ "wait_from_task", wait_from_task, 10, WAIT_FROM_TASK_TRAP },
};

static_assert(task::MAX_WORKERS == 16, "frames<N> cases cover 1 to MAX_WORKERS");

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
