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

__attribute__((constructor))
static void init() {
	id_thread = ID_MAIN;
}

}