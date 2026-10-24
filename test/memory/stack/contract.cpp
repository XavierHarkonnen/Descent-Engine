// Contract checks for memory::stack::Box: a Box is bound to the scope that
// receives it from Allocator, on the stack of the thread that created it, and
// must not outlive that thread.
//
// Valid use is checked in every build. Debug builds also check that each way a
// Box can leave its thread's stack, or outlive its thread, traps with a message
// naming the cause. Those checks compile out of release builds, where the
// misuse is undefined, so the trap cases are not run there.
//
// Misuse that must not compile is checked by box_misuse.cpp.

extern "C" {
#include <pthread.h>
#include <string.h>
}

#include <harness.hpp>

#include <descent/memory/stack.hpp>
#include <descent/type/core.hpp>

using namespace descent;
using namespace descent::memory;

using Box = stack::Box<u64>;

// Box adds no debug bookkeeping to its layout.
static_assert(sizeof(Box) == 4 * sizeof(u64), "Box is four words");

static Box make(u64 capacity, u64 value) {
	return stack::Allocator::create<u64>(capacity, value);
}

// Scoped use, including out-of-order frees and a function returning a Box
// prvalue. Returns false on the first unexpected result.
static bool scoped() {
	auto outer = stack::Allocator::create<u64>(64, u64(1));
	if (!outer)
		return false;

	for (u64 i = 0; i < 100; ++i) {
		auto a = stack::Allocator::create<u64>(i + 1, i);
		auto b = make(2 * i + 1, i + 1);
		{
			auto c = stack::Allocator::copy(string("scope"));
			if (!a || !b || !c || a[i] != i || b[0] != i + 1 || c[5] != '\0')
				return false;
		}
	}

	return outer[63] == 1;
}

static void *scoped_thread(void *) {
	return scoped() ? nullptr : reinterpret_cast<void *>(1);
}

static void scoped_main_thread() {
	test_check(scoped(), "scoped use on the main thread");
}

static void scoped_threads() {
	constexpr u32 THREADS = 8;

	pthread_t threads[THREADS];
	for (pthread_t &t : threads)
		test_require(pthread_create(&t, nullptr, scoped_thread, nullptr) == 0, "pthread_create");

	for (u32 i = 0; i < THREADS; ++i) {
		void *result;
		pthread_join(threads[i], &result);
		test_check(result == nullptr, "scoped use on thread %u of %u", i, THREADS);
	}
}

#if defined(DESCENT_DEBUG)

// The misuse cases construct a Box where Box forbids it. Box deletes its class
// operator new, so they use a global placement form with a test-only tag
// rather than the one in <new>.
struct Place {};

inline void *operator new(__SIZE_TYPE__, Place, void *p) {
	return p;
}

static void join(void *(*fn)(void *), void *arg = nullptr) {
	pthread_t t;
	test_require(pthread_create(&t, nullptr, fn, arg) == 0, "pthread_create");
	pthread_join(t, nullptr);
}

// Hands a Box from one thread to another, which destroys it.

static pthread_mutex_t gate = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static Box *shared = nullptr;

__attribute__((noreturn))
static void publish_and_wait(Box *box) {
	pthread_mutex_lock(&gate);
	shared = box;
	pthread_cond_broadcast(&cond);
	for (;;)
		pthread_cond_wait(&cond, &gate);
}

static Box *wait_for_shared() {
	pthread_mutex_lock(&gate);
	while (!shared)
		pthread_cond_wait(&cond, &gate);
	pthread_mutex_unlock(&gate);
	return shared;
}

static void *destroy_shared(void *) {
	shared->~Box();
	return nullptr;
}

static void *owner(void *) {
	Box box = stack::Allocator::create<u64>(16, u64(7));
	publish_and_wait(&box);
	return nullptr;
}

static void destroyed_on_other_thread() {
	pthread_t t;
	test_require(pthread_create(&t, nullptr, owner, nullptr) == 0, "pthread_create");
	wait_for_shared();
	join(destroy_shared);
}

// Box created outside a local variable of the calling thread.

alignas(Box) static unsigned char static_storage[sizeof(Box)];

static void created_in_static_storage() {
	::new (Place{}, static_storage) Box(stack::Allocator::create<u64>(16));
}

alignas(Box) static thread_local unsigned char thread_storage[sizeof(Box)];

static void created_in_thread_local_storage() {
	::new (Place{}, thread_storage) Box(stack::Allocator::create<u64>(16));
}

static void created_on_heap() {
	Box *box = ::new Box(stack::Allocator::create<u64>(16));
	(void) box;
}

// Box still alive when its thread exits.

static void *leak_on_stack(void *) {
	alignas(Box) unsigned char storage[sizeof(Box)];
	::new (Place{}, storage) Box(stack::Allocator::create<u64>(16));
	return nullptr;
}

static void outlives_thread() {
	join(leak_on_stack);
}

// A Box's bytes copied onto another thread's stack and destroyed there. This
// reaches the allocator's own check, behind the stack check.

alignas(Box) static unsigned char bytes[sizeof(Box)];

static void *copy_out(void *) {
	auto box = stack::Allocator::create<u64>(16);
	memcpy(bytes, static_cast<void *>(&box), sizeof(Box));

	// Abandon the original so only the copy frees the allocation.
	alignas(Box) unsigned char moved[sizeof(Box)];
	::new (Place{}, moved) Box(stack::Allocator::create<u64>(1));
	memcpy(static_cast<void *>(&box), moved, sizeof(Box));
	return nullptr;
}

static void *destroy_copy(void *) {
	alignas(Box) unsigned char storage[sizeof(Box)];
	memcpy(storage, bytes, sizeof(Box));
	reinterpret_cast<Box *>(storage)->~Box();
	return nullptr;
}

static void *copy_then_wait(void *) {
	copy_out(nullptr);
	publish_and_wait(reinterpret_cast<Box *>(bytes));
	return nullptr;
}

static void freed_into_other_allocator() {
	pthread_t t;
	test_require(pthread_create(&t, nullptr, copy_then_wait, nullptr) == 0, "pthread_create");
	wait_for_shared();
	join(destroy_copy);
}

static constexpr const char *NOT_LOCAL = "not a local variable of the calling thread";

#endif

static constexpr test::Case cases[] = {
	{ "scoped_main_thread", scoped_main_thread },
	{ "scoped_threads",     scoped_threads },
#if defined(DESCENT_DEBUG)
	{ "destroyed_on_other_thread",       destroyed_on_other_thread,       10, NOT_LOCAL },
	{ "created_in_static_storage",       created_in_static_storage,       10, NOT_LOCAL },
	{ "created_in_thread_local_storage", created_in_thread_local_storage, 10, NOT_LOCAL },
	{ "created_on_heap",                 created_on_heap,                 10, NOT_LOCAL },
	{ "outlives_thread",                 outlives_thread,                 10,
		"Thread exited while a Box it allocated was alive" },
	{ "freed_into_other_allocator",      freed_into_other_allocator,      10,
		"freed on a thread other than the one that allocated it" },
#endif
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
