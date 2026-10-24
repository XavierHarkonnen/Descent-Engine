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

#include <descent/rengine/rengine.hpp>

extern "C" {
#include <vulkan/vulkan.h>
#include <wayland-client.h>
}

namespace descent::rengine {

Rengine::Rengine() : _init(false) {
	_display = wl_display_connect(nullptr);
	if (!_display)
		return;

	_init = true;
}

Rengine::~Rengine() {
	if (_display)
		wl_display_disconnect(_display);
}

Rengine::operator bool() {
	return _init;
}

}
