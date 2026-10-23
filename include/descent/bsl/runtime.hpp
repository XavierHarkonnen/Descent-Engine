#ifndef DESCENT_RUNTIME_HPP
#define DESCENT_RUNTIME_HPP

#include <descent/bsl/level.hpp>

namespace descent::bsl {

static constexpr Level LEVEL = Level::DEFAULT;

void enable(descent::bsl::Level level);

void disable(descent::bsl::Level level);

// Return all currently enabled levels (compile and runtime)
Level enabled();

// Test if a level is enabled (compile and runtime)
static inline bool enabled(Level level) {
	return bool(enabled() & level);
}

// Initialize logging for thread
bool init_thread();

}

#endif