#include <descent/memory.h>

#include <sys/mman.h>
#include <unistd.h>

#include <descent/sys.h>
#include <descent/types.h>

static u64 ALLOC_GRANULARITY;
static u64 ALLOC_GRANULARITY_MASK;

static inline u64 sys_round_size(u64 size) {
	return (size + ALLOC_GRANULARITY - 1) & ~(ALLOC_GRANULARITY - 1);
}

static inline bool sys_alloc_is_aligned(void *alloc) {
	return ((u64) alloc & (ALLOC_GRANULARITY - 1)) == 0;
}

static inline bool sys_size_is_aligned(u64 size) {
	return (size & (ALLOC_GRANULARITY - 1)) == 0;
}

static inline int sys_prot(enum mem_prot prot) {
	switch (prot) {
		case MEM_NONE: return PROT_NONE;
		case MEM_READ: return PROT_READ;
		case MEM_EDIT: return PROT_READ | PROT_WRITE;
	}

	return -1;
}

__attribute__((constructor))
static void sys_init(void) {
	long result = sysconf(_SC_PAGESIZE);

	if (result <= 0)
		sys_fatal("Allocation granularity could not be determined");

	if ((result & (result - 1)) != 0)
		sys_fatal("Allocation granularity is not a power of two");

	ALLOC_GRANULARITY = (u64) result;
	ALLOC_GRANULARITY_MASK = ALLOC_GRANULARITY - 1;
}

u64 sys_alloc_granularity(void) {
	return ALLOC_GRANULARITY;
}

void *sys_alloc(u64 *size, enum mem_prot prot) {
	if (!size)
		return NULL;

	*size = sys_round_size(*size);
	if (*size == 0)
		return NULL;

	if (sys_prot(prot) < 0) {
		*size = 0;
		return NULL;
	}

	void *alloc = mmap(NULL, *size, sys_prot(prot), MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (alloc == MAP_FAILED) {
		*size = 0;
		return NULL;
	}

	return alloc;
}

bool sys_protect(void *alloc, u64 size, enum mem_prot prot) {
	if (!alloc || !size)
		return false;

	if (!sys_alloc_is_aligned(alloc))
		return false;
	
	if (!sys_size_is_aligned(size))
		return false;
	
	if (sys_prot(prot) < 0)
		return false;

	if (mprotect(alloc, size, sys_prot(prot)))
		return false;

	return true;
}

bool sys_free(void *alloc, u64 size) {
	if (!alloc || !size)
		return false;

	if (!sys_alloc_is_aligned(alloc))
		return false;
	
	if (!sys_size_is_aligned(size))
		return false;
	
	if (munmap(alloc, size))
		return false;

	return true;
}
