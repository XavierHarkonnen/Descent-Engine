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

#ifndef DESCENT_BSL_RUNTIME_HPP
#define DESCENT_BSL_RUNTIME_HPP

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
