#include <unistd.h>

#include <descent/type/core.h>
#include <descent/thread/mutex.h>

struct thread_data {
	void *stack; 
	pid_t pid;
};

struct thread_pool {
	u8 count;
	struct thread_data data[THREAD_POOL_CAPACITY];
};

struct thread_pool_registry {
	struct thread_meta meta[MAX_THREADS];
	struct mutex lock;
	u64 free_head;
	u8 free_stack[MAX_THREADS];
};

static _Thread_local struct thread_data *self = NULL;

bool thread_is_main(void) {
	return self == NULL;
}

typedef bool (*thread_pool_function)(void *);

u32 thread_pool_create(thread_pool_function function, void *argument);