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

#include <stdio.h>

#include <descent/rengine.h>
#include <descent/metadata.h>
#include <descent/types/core.h>

int main() {
	struct rengine_create_info info = {0};
	info.debug = DEBUG_VALIDATION;
	info.name = "Granbarrow";
	info.version = MAKE_VERSION(0, 0, 0, 1);
	info.width = 800;
	info.height = 600;

	u64 result = rengine_initialize(&info);
	if (result) {
		printf("INITIALIZE ERROR: %s\n", rengine_result_string(result));
		return -1;
	}

	//while (!glfwWindowShouldClose(context->window)) {
	//	glfwPollEvents();
	//}

	result = rengine_terminate();
	if (result) {
		printf("TERMINATE ERROR: %s\n", rengine_result_string(result));
		return -1;
	}

	return 0;
}

/* Goals:
- To be as disgustingly x64/linux-centric as possible

GLFW
Vulkan

futexes
vectored IO
io uring

// Read configuration
// Initialize
// Create graphics context
*/
