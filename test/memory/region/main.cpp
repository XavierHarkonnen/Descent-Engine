// memory::Region with a page count chosen at run time (DYNAMIC_PAGES), for an
// element type other than u8. Its storage was once typed as u8 * (fixed in
// 75fa96d). Resizing is not covered while QA-02 is open.

#include <harness.hpp>

#include <descent/memory/platform.hpp>
#include <descent/memory/region.hpp>
#include <descent/type/core.hpp>

using namespace descent;
using namespace descent::memory;

static void dynamic_non_byte_elements() {
	Region<u32> region;
	test_check(!region, "a default-constructed Region holds storage");

	test_require(region.acquire(1), "acquire(1)");
	test_check(bool(region), "an acquired Region reports no storage");
	test_check(region.pages() == 1, "pages() = %u, expected 1", region.pages());
	test_require(region.capacity() == PAGE_SIZE / sizeof(u32), "capacity() = %llu, expected %llu",
		(unsigned long long) region.capacity(), (unsigned long long) (PAGE_SIZE / sizeof(u32)));

	for (u64 i = 0; i < region.capacity(); ++i)
		region[i] = u32(i * 2654435761u);

	u64 wrong = 0;
	for (u64 i = 0; i < region.capacity(); ++i)
		wrong += region[i] != u32(i * 2654435761u);
	test_check(wrong == 0, "%llu of %llu elements read back wrong", (unsigned long long) wrong,
		(unsigned long long) region.capacity());

	// Elements are distinct u32 objects, one sizeof(u32) apart.
	test_check(reinterpret_cast<u8 *>(&region[1]) - reinterpret_cast<u8 *>(&region[0]) == sizeof(u32),
		"elements are not sizeof(u32) apart");

	region.release();
	test_check(!region, "a released Region holds storage");
}

static constexpr test::Case cases[] = {
	{ "dynamic_non_byte_elements", dynamic_non_byte_elements },
};

int main(int argc, char **argv) {
	return test::run(cases, argc, argv);
}
